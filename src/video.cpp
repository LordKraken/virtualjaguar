//
// VIDEO.CPP: SDL/local hardware specific video routines
//
// by James L. Hammons
//

#include "types.h"
#include "tom.h"
#include "sdlemu_opengl.h"
#include "settings.h"
#include "video.h"

// External global variables

//shouldn't these exist here??? Prolly.
extern SDL_Surface * surface, * mainSurface;
extern Uint32 mainSurfaceFlags;
extern int16 * backbuffer;
extern SDL_Joystick * joystick;

//
// Prime SDL and create surfaces
//
bool InitVideo(void)
{
	// Set up SDL library
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0)
	{
		WriteLog("VJ: Could not initialize the SDL library: %s", SDL_GetError());
		return false;
	}

	// Get proper info about the platform we're running on...
	const SDL_VideoInfo * info = SDL_GetVideoInfo();

	if (!info)
	{
		WriteLog("VJ: SDL is unable to get the video info: %s\n", SDL_GetError());
		return false;
	}

	if (vjs.useOpenGL)
	{
		mainSurfaceFlags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	}
	else
	{
		if (info->hw_available)
			mainSurfaceFlags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;

		if (info->blit_hw)
			mainSurfaceFlags |= SDL_HWACCEL;
	}

	if (vjs.fullscreen)
		mainSurfaceFlags |= SDL_FULLSCREEN;

	if (!vjs.useOpenGL)
		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT, 16, mainSurfaceFlags);
	else
		mainSurface = SDL_SetVideoMode(VIRTUAL_SCREEN_WIDTH * 2, VIRTUAL_SCREEN_HEIGHT * 2, 16, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("VJ: SDL is unable to set the video mode: %s\n", SDL_GetError());
		return false;
	}

	SDL_WM_SetCaption("Virtual Jaguar", "Virtual Jaguar");

	// Create the primary SDL display (16 BPP, 5/5/5 RGB format)
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT,
		16, 0x7C00, 0x03E0, 0x001F, 0);

	if (surface == NULL)
	{
		WriteLog("VJ: Could not create primary SDL surface: %s\n", SDL_GetError());
		return false;
	}

	if (vjs.useOpenGL)
		sdlemu_init_opengl(surface, 1/*method*/, 2/*size*/, 0/*texture type (linear, nearest)*/);

	// Initialize Joystick support under SDL
	if (vjs.useJoystick)
	{
		if (SDL_NumJoysticks() <= 0)
		{
			vjs.useJoystick = false;
			printf("VJ: No joystick(s) or joypad(s) detected on your system. Using keyboard...\n");
		}
		else
		{
			if ((joystick = SDL_JoystickOpen(vjs.joyport)) == 0)
			{
				vjs.useJoystick = false;
				printf("VJ: Unable to open a Joystick on port: %d\n", (int)vjs.joyport);
			}
			else
				printf("VJ: Using: %s\n", SDL_JoystickName(vjs.joyport));
		}
	}

	return true;
}

//
// Free various SDL components
//
void VideoDone(void)
{
	if (vjs.useOpenGL)
		sdlemu_close_opengl();

	SDL_JoystickClose(joystick);
	SDL_FreeSurface(surface);
	SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	SDL_Quit();
}

//
// Render the backbuffer to the primary screen surface
//
void RenderBackbuffer(void)
{
	if (SDL_MUSTLOCK(surface))
		while (SDL_LockSurface(surface) < 0)
			SDL_Delay(10);

	memcpy(surface->pixels, backbuffer, tom_getVideoModeWidth() * tom_getVideoModeHeight() * 2);

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);

	if (vjs.useOpenGL)
		sdlemu_draw_texture(mainSurface, surface, 1/*1=GL_QUADS*/);
	else
	{
		SDL_Rect rect = { 0, 0, surface->w, surface->h };
		SDL_BlitSurface(surface, &rect, mainSurface, &rect);
		SDL_Flip(mainSurface);
    }
}

//
// Resize the main SDL screen & backbuffer
//
void ResizeScreen(uint32 width, uint32 height)
{
	char window_title[256];

	SDL_FreeSurface(surface);
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16,
		0x7C00, 0x03E0, 0x001F, 0);

	if (surface == NULL)
	{
		WriteLog("Video: Could not create primary SDL surface: %s", SDL_GetError());
		exit(1);
	}

	sprintf(window_title, "Virtual Jaguar (%i x %i)", (int)width, (int)height);

	if (!vjs.useOpenGL)
	{
		mainSurface = SDL_SetVideoMode(width, height, 16, mainSurfaceFlags);

		if (mainSurface == NULL)
		{
			WriteLog("Video: SDL is unable to set the video mode: %s\n", SDL_GetError());
			exit(1);
		}
	}

	SDL_WM_SetCaption(window_title, window_title);

	// This seems to work well for resizing (i.e., changes in the pixel width)...
	if (vjs.useOpenGL)
		sdlemu_resize_texture(surface, mainSurface);
}

//
// Return the screen's pitch
//
uint32 GetSDLScreenPitch(void)
{
	return surface->pitch;
}

//
// Fullscreen <-> window switching
//
//NOTE: This does *NOT* work with OpenGL rendering! !!! FIX !!!
void ToggleFullscreen(void)
{
	vjs.fullscreen = !vjs.fullscreen;
	mainSurfaceFlags &= ~SDL_FULLSCREEN;

	if (vjs.fullscreen)
		mainSurfaceFlags |= SDL_FULLSCREEN;

	mainSurface = SDL_SetVideoMode(tom_width, tom_height, 16, mainSurfaceFlags);

	if (mainSurface == NULL)
	{
		WriteLog("Video: SDL is unable to set the video mode: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_WM_SetCaption("Virtual Jaguar", "Virtual Jaguar");
}