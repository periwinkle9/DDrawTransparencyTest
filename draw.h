#pragma once

#include "framework.h"
#include <ddraw.h>

constexpr int WINDOW_WIDTH = 320;
constexpr int WINDOW_HEIGHT = 240;

extern LPDIRECTDRAW lpDD;
extern LPDIRECTDRAWSURFACE frontbuffer;
extern LPDIRECTDRAWSURFACE backbuffer;
extern LPDIRECTDRAWSURFACE roachSurface;
extern LPDIRECTDRAWSURFACE roachSurfaceScaled;
extern LPDIRECTDRAWSURFACE frogSurface1;
extern LPDIRECTDRAWSURFACE frogSurface2;
extern LPDIRECTDRAWCLIPPER clipper;
extern RECT windowRect;

BOOL initDDraw(HWND hWnd);
void releaseDDraw();
BOOL renderLoop(HINSTANCE hInstance, HWND hWnd);
