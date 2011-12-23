//
// DAC (really, Synchronous Serial Interface) Handler
//
// Originally by David Raingeard
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Rewritten by James L. Hammons
//

// Need to set up defaults that the BIOS sets for the SSI here in DACInit()... !!! FIX !!!
// or something like that... Seems like it already does, but it doesn't seem to
// work correctly...! Perhaps just need to set up SSI stuff so BUTCH doesn't get
// confused...

#include "SDL.h"
#include "m68k.h"
#include "jaguar.h"
#include "settings.h"
#include "dac.h"

//#define DAC_DEBUG

#define BUFFER_SIZE		0x10000						// Make the DAC buffers 64K x 16 bits

// Jaguar memory locations

/*#define LTXD			0xF1A148
#define RTXD			0xF1A14C
#define LRXD			0xF1A148
#define RRXD			0xF1A14C
#define SCLK			0xF1A150
#define SMODE			0xF1A154
*/
// Local variables

//static uint32 LeftFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOHeadPtr, RightFIFOTailPtr;
static SDL_AudioSpec desired;
static bool SDLSoundInitialized = false;

// We can get away with using native endian here because we can tell SDL to use the native
// endian when looking at the sample buffer, i.e., no need to worry about it.

uint16 * DACBuffer;							// Left and right audio data
uint16 DACBufferIndex;
//static uint8 SCLKFrequencyDivider = 19;				// Default is roughly 22 KHz (20774 Hz in NTSC mode)
//static uint16 serialMode = 0;

extern uint16 I2S_SerialClock;

// Private function prototypes

void SDLSoundCallback(void * userdata, Uint8 * buffer, int length);
int GetCalculatedFrequency(void);
void SDLAdjustAudioFrequency(void);

//
// Initialize the SDL sound system
//
void DACInit(void)
{
	memory_malloc_secure((void **)&DACBuffer, BUFFER_SIZE * sizeof(uint16), "DAC buffer");

	DACBufferIndex = 0;

	desired.freq = GetCalculatedFrequency();		// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
	desired.format = AUDIO_S16SYS;					// This uses the native endian (for portability)...
	desired.channels = 2;
//	desired.samples = 4096;							// Let's try a 4K buffer (can always go lower)
	desired.samples = 2048;							// Let's try a 2K buffer (can always go lower)
	desired.callback = SDLSoundCallback;

	if (SDL_OpenAudio(&desired, NULL) < 0)			// NULL means SDL guarantees what we want
		WriteLog("DAC: Failed to initialize SDL sound...\n");
	else
	{
		SDLSoundInitialized = true;
		DACReset();
		SDL_PauseAudio(false);							// Start playback!
		WriteLog("DAC: Successfully initialized.\n");
	}
}

//
// Reset the sound buffer FIFOs
//
void DACReset(void)
{
	DACBufferIndex = 0;
	//LeftFIFOHeadPtr = LeftFIFOTailPtr = 0, RightFIFOHeadPtr = RightFIFOTailPtr = 1;
}

//
// Close down the SDL sound subsystem
//
void DACDone(void)
{
	if (SDLSoundInitialized)
	{
		SDL_PauseAudio(true);
		SDL_CloseAudio();
	}

	memory_free(DACBuffer);
	WriteLog("DAC: Done.\n");
}

//
// SDL callback routine to fill audio buffer
//
// Note: The samples are packed in the buffer in 16 bit left/16 bit right pairs.
//
void SDLSoundCallback(void * userdata, Uint8 * buffer, int length)
{
	if (DACBufferIndex > 0)
	{
		//WriteLog("DAC: Index: %d, Bufferlength: %d\n", DACBufferIndex, length);
		if (DACBufferIndex > length)
		{
			//SDL_MixAudio(buffer, (uint8*)&DACBuffer, length, SDL_MIX_MAXVOLUME);
			memcpy(buffer, DACBuffer, length);
			// Shift remaining content to start of buffer
			DACBufferIndex = DACBufferIndex - length;
			memcpy(DACBuffer, DACBuffer + length, DACBufferIndex);
		}
		else
		{		
			//SDL_MixAudio(buffer, (uint8*)&DACBuffer, DACBufferIndex, SDL_MIX_MAXVOLUME);
			memcpy(buffer, DACBuffer, DACBufferIndex);
			DACBufferIndex = 0;
		}
	}
	else
		memset(buffer, desired.silence, length);
}
/*
// Clear the buffer to silence, in case the DAC buffer is empty (or short)
//This causes choppy sound... Ick.
	//memset(buffer, desired.silence, length);

	//WriteLog("DAC: Inside callback...\n");
//	if (LeftFIFOHeadPtr != LeftFIFOTailPtr)
	{
//WriteLog("DAC: About to write some data!\n");
		int numLeftSamplesReady
			= (LeftFIFOTailPtr + (LeftFIFOTailPtr < LeftFIFOHeadPtr ? BUFFER_SIZE : 0))
				- LeftFIFOHeadPtr;
		int numRightSamplesReady
			= (RightFIFOTailPtr + (RightFIFOTailPtr < RightFIFOHeadPtr ? BUFFER_SIZE : 0))
				- RightFIFOHeadPtr;
//This waits for the slower side to catch up. If writing only one side, then this
//causes the buffer not to drain...
		int numSamplesReady
			= (numLeftSamplesReady < numRightSamplesReady
				? numLeftSamplesReady : numRightSamplesReady);//Hmm. * 2;

//Kludge, until I can figure out WTF is going on WRT Super Burnout.
if (numLeftSamplesReady == 0 || numRightSamplesReady == 0)
	numSamplesReady = numLeftSamplesReady + numRightSamplesReady;

//The numbers look good--it's just that the DSP can't get enough samples in the DAC buffer!
//WriteLog("DAC: Left/RightFIFOHeadPtr: %u/%u, Left/RightFIFOTailPtr: %u/%u\n", LeftFIFOHeadPtr, RightFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOTailPtr);
//WriteLog("     numLeft/RightSamplesReady: %i/%i, numSamplesReady: %i, length of buffer: %i\n", numLeftSamplesReady, numRightSamplesReady, numSamplesReady, length);

//		if (numSamplesReady > length)
//			numSamplesReady = length;
		if (numSamplesReady > length / 2)	// length / 2 because we're comparing 16-bit lengths
			numSamplesReady = length / 2;
//else
//	WriteLog("     Not enough samples to fill the buffer (short by %u L/R samples)...\n", (length / 2) - numSamplesReady);
//WriteLog("DAC: %u samples ready.\n", numSamplesReady);

		// Actually, it's a bit more involved than this, but this is the general idea:
//		memcpy(buffer, DACBuffer, length);
//numSamplesReady = 2;
//		for(int i=0; i<numSamplesReady; i++)
//			((uint16 *)buffer)[i] = rand() & 0xFFFF;//DACBuffer[(LeftFIFOHeadPtr + i) % BUFFER_SIZE];

		// Could also use (as long as BUFFER_SIZE is a multiple of 2):
//			buffer[i] = DACBuffer[(LeftFIFOHeadPtr + i) & (BUFFER_SIZE - 1)];

		LeftFIFOHeadPtr = (LeftFIFOHeadPtr + numSamplesReady) % BUFFER_SIZE;
		RightFIFOHeadPtr = (RightFIFOHeadPtr + numSamplesReady) % BUFFER_SIZE;
		// Could also use (as long as BUFFER_SIZE is a multiple of 2):
//		LeftFIFOHeadPtr = (LeftFIFOHeadPtr + numSamplesReady) & (BUFFER_SIZE - 1);
//		RightFIFOHeadPtr = (RightFIFOHeadPtr + numSamplesReady) & (BUFFER_SIZE - 1);
//WriteLog("  -> Left/RightFIFOHeadPtr: %04X/%04X, Left/RightFIFOTailPtr: %04X/%04X\n", LeftFIFOHeadPtr, RightFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOTailPtr);
	}
//Hmm. Seems that the SDL buffer isn't being starved by the DAC buffer...
//	else
//		WriteLog("DAC: Silence...!\n");
}*/

//
// Calculate the frequency of SCLK * 32 using the divider
//
int GetCalculatedFrequency(void)
{
	int systemClockFrequency = (vjs.hardwareTypeNTSC ? RISC_CLOCK_RATE_NTSC : RISC_CLOCK_RATE_PAL);

	// We divide by 32 here in order to find the frequency of 32 SCLKs in a row (transferring
	// 16 bits of left data + 16 bits of right data = 32 bits, 1 SCLK = 1 bit transferred).
	return systemClockFrequency / (32 * (2 * (I2S_SerialClock + 1)));
}

//
// Adjust the SDL audio frequency
//
void SDLAdjustAudioFrequency(void)
{
	//Of course a better way would be to query the hardware to find the upper limit...
	if (I2S_SerialClock > 7)	// Anything less than 8 is too high! ???
	{
		if (SDLSoundInitialized)
			SDL_CloseAudio();

		desired.freq = GetCalculatedFrequency();// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
		WriteLog("DAC: Changing sample rate to %u Hz!\n", desired.freq);

		if (SDLSoundInitialized)
		{
			if (SDL_OpenAudio(&desired, NULL) < 0)	// NULL means SDL guarantees what we want
			{
				WriteLog("DAC: Failed to initialize SDL sound: %s.\nDesired freq: %u\nShutting down!\n", SDL_GetError(), desired.freq);
				log_done();
				exit(1);
			}
		}
		DACReset();

		if (SDLSoundInitialized)
			SDL_PauseAudio(false);			// Start playback!
	}
}

//
// LTXD/RTXD/SCLK/SMODE ($F1A148/4C/50/54)
//
/*void DACWriteByte(uint32 address, uint8 data, uint32 who)
{
	WriteLog("DAC: %s writing BYTE %02X at %08X\n", whoName[who], data, address);
	if (address == (SCLK + 3))
		DACWriteWord(address - 3, (uint16)data);
}*/

/*void DACWriteWord(uint32 address, uint16 data, uint32 who)
{
	if (address == (LTXD + 2))
	{
		// Spin until buffer has been drained (for too fast processors!)...
//Small problem--if Head == 0 and Tail == buffer end, then this will fail... !!! FIX !!!
//[DONE]
		// Also, we're taking advantage of the fact that the buffer is a multiple of two
		// in this check...
uint32 spin = 0;
		while (((LeftFIFOTailPtr + 2) & (BUFFER_SIZE - 1)) == LeftFIFOHeadPtr)//;
		{
spin++;
//if ((spin & 0x0FFFFFFF) == 0)
//	WriteLog("Tail=%X, Head=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);

if (spin == 0xFFFF0000)
{
uint32 ltail = LeftFIFOTailPtr, lhead = LeftFIFOHeadPtr;
WriteLog("Tail=%X, Head=%X", ltail, lhead);

	WriteLog("\nStuck in left DAC spinlock! Aborting!\n");
	WriteLog("LTail=%X, LHead=%X, BUFFER_SIZE-1=%X\n", LeftFIFOTailPtr, LeftFIFOHeadPtr, BUFFER_SIZE - 1);
	WriteLog("RTail=%X, RHead=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);
	WriteLog("From while: Tail=%X, Head=%X", (LeftFIFOTailPtr + 2) & (BUFFER_SIZE - 1), LeftFIFOHeadPtr);
	log_done();
	exit(0);
}
		}

		SDL_LockAudio();							// Is it necessary to do this? Mebbe.
		// We use a circular buffer 'cause it's easy. Note that the callback function
		// takes care of dumping audio to the soundcard...! Also note that we're writing
		// the samples in the buffer in an interleaved L/R format.
		LeftFIFOTailPtr = (LeftFIFOTailPtr + 2) % BUFFER_SIZE;
		DACBuffer[LeftFIFOTailPtr] = data;
		SDL_UnlockAudio();
	}
	else if (address == (RTXD + 2))
	{
/*
Here's what's happening now:

Stuck in right DAC spinlock!
Aborting!

Tail=681, Head=681, BUFFER_SIZE-1=FFFF
From while: Tail=683, Head=681

????? What the FUCK ?????

& when I uncomment the lines below spin++; it *doesn't* lock here... WTF?????

I think it was missing parentheses causing the fuckup... Seems to work now...

Except for Super Burnout now...! Aarrrgggghhhhh!

Tail=AC, Head=AE
Stuck in left DAC spinlock! Aborting!
Tail=AC, Head=AE, BUFFER_SIZE-1=FFFF
From while: Tail=AE, Head=AE

So it's *really* stuck here in the left FIFO. Figure out why!!!

Prolly 'cause it doesn't set the sample rate right away--betcha it works with the BIOS...
It gets farther, but then locks here (weird!):

Tail=2564, Head=2566
Stuck in left DAC spinlock! Aborting!
Tail=2564, Head=2566, BUFFER_SIZE-1=FFFF
From while: Tail=2566, Head=2566

Weird--recompile with more WriteLog() entries and it *doesn't* lock...
Yeah, because there was no DSP running. Duh!

Tail=AC, Head=AE
Stuck in left DAC spinlock! Aborting!
LTail=AC, LHead=AE, BUFFER_SIZE-1=FFFF
RTail=AF, RHead=AF, BUFFER_SIZE-1=FFFF
From while: Tail=AE, Head=AE

Odd: The right FIFO is empty, but the left FIFO is full!
And this is what is causing the lockup--the DAC callback waits for the side with
less samples ready and in this case it's the right channel (that never fills up)
that it's waiting for...!

Okay, with the kludge in place for the right channel not being filled, we select
a track and then it locks here:

Tail=60D8, Head=60DA
Stuck in left DAC spinlock! Aborting!
LTail=60D8, LHead=60D8, BUFFER_SIZE-1=FFFF
RTail=DB, RHead=60D9, BUFFER_SIZE-1=FFFF
From while: Tail=60DA, Head=60D8
*/
/*		// Spin until buffer has been drained (for too fast processors!)...
uint32 spin = 0;
		while (((RightFIFOTailPtr + 2) & (BUFFER_SIZE - 1)) == RightFIFOHeadPtr)//;
		{
spin++;
//if ((spin & 0x0FFFFFFF) == 0)
//	WriteLog("Tail=%X, Head=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);

if (spin == 0xFFFF0000)
{
uint32 rtail = RightFIFOTailPtr, rhead = RightFIFOHeadPtr;
WriteLog("Tail=%X, Head=%X", rtail, rhead);

	WriteLog("\nStuck in right DAC spinlock! Aborting!\n");
	WriteLog("LTail=%X, LHead=%X, BUFFER_SIZE-1=%X\n", LeftFIFOTailPtr, LeftFIFOHeadPtr, BUFFER_SIZE - 1);
	WriteLog("RTail=%X, RHead=%X, BUFFER_SIZE-1=%X\n", RightFIFOTailPtr, RightFIFOHeadPtr, BUFFER_SIZE - 1);
	WriteLog("From while: Tail=%X, Head=%X", (RightFIFOTailPtr + 2) & (BUFFER_SIZE - 1), RightFIFOHeadPtr);
	log_done();
	exit(0);
}
		}

		SDL_LockAudio();
		RightFIFOTailPtr = (RightFIFOTailPtr + 2) % BUFFER_SIZE;
		DACBuffer[RightFIFOTailPtr] = data;
		SDL_UnlockAudio();
/*#ifdef DAC_DEBUG
		else
			WriteLog("DAC: Ran into FIFO's right tail pointer!\n");
#endif*/
/*	}
	else if (address == (SCLK + 2))					// Sample rate
	{
		WriteLog("DAC: Writing %u to SCLK...\n", data);
		if ((uint8)data != SCLKFrequencyDivider)
		{
			SCLKFrequencyDivider = (uint8)data;
//Of course a better way would be to query the hardware to find the upper limit...
			if (data > 7)	// Anything less than 8 is too high!
			{
				if (SDLSoundInitialized)
					SDL_CloseAudio();

				desired.freq = GetCalculatedFrequency();// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
				WriteLog("DAC: Changing sample rate to %u Hz!\n", desired.freq);

				if (SDLSoundInitialized)
				{
					if (SDL_OpenAudio(&desired, NULL) < 0)	// NULL means SDL guarantees what we want
					{
						WriteLog("DAC: Failed to initialize SDL sound: %s.\nDesired freq: %u\nShutting down!\n", SDL_GetError(), desired.freq);
						log_done();
						exit(1);
					}
				}

				DACReset();

				if (SDLSoundInitialized)
					SDL_PauseAudio(false);			// Start playback!
			}
		}
	}
}*/

//
// LRXD/RRXD/SSTAT ($F1A148/4C/50)
//
uint8 DACReadByte(uint32 address, uint32 who/*= UNKNOWN*/)
{
//	WriteLog("DAC: %s reading byte from %08X\n", whoName[who], address);
	return 0xFF;
}
