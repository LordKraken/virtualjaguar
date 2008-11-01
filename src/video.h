//
// VIDEO.H: Header file
//

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "types.h"

#define VIRTUAL_SCREEN_WIDTH		320
#define VIRTUAL_SCREEN_HEIGHT_NTSC	240
#define VIRTUAL_SCREEN_HEIGHT_PAL	256

bool VideoInit(void);
void VideoDone(void);
void RenderBackbuffer(void);
void ResizeScreen(uint32 width, uint32 height);
uint32 GetSDLScreenWidthInPixels(void);
void ToggleFullscreen(void);

// Exported vars

extern uint32 * backbuffer;

#endif	// __VIDEO_H__
