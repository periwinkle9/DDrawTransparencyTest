#include "draw.h"
#include "DDrawTransparencyTest.h"
#include "log.h"

// For timeGetTime()
#include <MMSystem.h>

#include <stdio.h>

LPDIRECTDRAW lpDD;
LPDIRECTDRAWSURFACE frontbuffer;
LPDIRECTDRAWSURFACE backbuffer;
LPDIRECTDRAWSURFACE roachSurface;
LPDIRECTDRAWSURFACE roachSurfaceScaled;
LPDIRECTDRAWSURFACE frogSurface1;
LPDIRECTDRAWSURFACE frogSurface2;
LPDIRECTDRAWCLIPPER clipper;
RECT windowRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

BOOL initDDraw(HWND hWnd)
{
	// Set up DirectDraw like Doukutsu
	HRESULT result;

	if ((result = DirectDrawCreate(NULL, &lpDD, NULL)) != DD_OK)
	{
		reportFailure("DirectDrawCreate", __LINE__, result);
		return FALSE;
	}
	if ((result = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL)) != DD_OK)
	{
		reportFailure("lpDD->SetCooperativeLevel", __LINE__, result);
		return FALSE;
	}

	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if ((result = lpDD->CreateSurface(&ddsd, &frontbuffer, NULL)) != DD_OK)
	{
		reportFailure("lpDD->CreateSurface", __LINE__, result);
		return FALSE;
	}

	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = WINDOW_WIDTH;
	ddsd.dwHeight = WINDOW_HEIGHT;

	if ((result = lpDD->CreateSurface(&ddsd, &backbuffer, NULL)) != DD_OK)
	{
		reportFailure("lpDD->CreateSurface", __LINE__, result);
		return FALSE;
	}

	if ((result = lpDD->CreateClipper(0, &clipper, NULL)) != DD_OK)
	{
		reportFailure("lpDD->CreateClipper", __LINE__, result);
		return FALSE;
	}
	if ((result = clipper->SetHWnd(0, hWnd)) != DD_OK)
	{
		reportFailure("clipper->SetHWnd", __LINE__, result);
		return FALSE;
	}
	if (frontbuffer->SetClipper(clipper) != DD_OK)
	{
		reportFailure("frontbuffer->SetClipper", __LINE__, result);
		return FALSE;
	}

	return TRUE;
}

void releaseDDraw()
{
	if (roachSurface != NULL)
	{
		roachSurface->Release();
		roachSurface = NULL;
	}
	if (roachSurfaceScaled != NULL)
	{
		roachSurfaceScaled->Release();
		roachSurfaceScaled = NULL;
	}
	if (frogSurface1 != NULL)
	{
		frogSurface1->Release();
		frogSurface1 = NULL;
	}
	if (frogSurface2 != NULL)
	{
		frogSurface2->Release();
		frogSurface2 = NULL;
	}
	if (frontbuffer != NULL)
	{
		frontbuffer->Release();
		frontbuffer = NULL;
		// Not sure why backbuffer->Release() isn't called?
		backbuffer = NULL;
	}

	if (lpDD != NULL)
	{
		lpDD->Release();
		lpDD = NULL;
	}
}

void restoreSurfaces()
{
	if (frontbuffer != NULL && frontbuffer->IsLost() == DDERR_SURFACELOST)
		frontbuffer->Restore();
	if (backbuffer != NULL && backbuffer->IsLost() == DDERR_SURFACELOST)
		backbuffer->Restore();
	if (roachSurface != NULL && roachSurface->IsLost() == DDERR_SURFACELOST)
		roachSurface->Restore();
	if (roachSurfaceScaled != NULL && roachSurfaceScaled->IsLost() == DDERR_SURFACELOST)
		roachSurfaceScaled->Restore();
	if (frogSurface1 != NULL && frogSurface1->IsLost() == DDERR_SURFACELOST)
		frogSurface1->Restore();
	if (frogSurface2 != NULL && frogSurface2->IsLost() == DDERR_SURFACELOST)
		frogSurface2->Restore();
}

std::string getSurfaceDescString(const DDSURFACEDESC& surfaceDesc)
{
	std::string messageString;
	auto write = [&messageString](const char* formatString, auto... args)
	{
		// I don't have C++20 std::format
		char str[200] = {};
		sprintf_s(str, sizeof str, formatString, args...);
		messageString += str;
	};

	DWORD surfaceFlags = surfaceDesc.dwFlags;
	write("flags: %X\n", surfaceFlags);

	if (surfaceFlags & DDSD_WIDTH)
		write("width: %u\n", surfaceDesc.dwWidth);
	if (surfaceFlags & DDSD_HEIGHT)
		write("height: %u\n", surfaceDesc.dwHeight);
	if (surfaceFlags & DDSD_ALPHABITDEPTH)
		write("alpha bit depth: %u\n", surfaceDesc.dwAlphaBitDepth);
	if (surfaceFlags & DDSD_BACKBUFFERCOUNT)
		write("backbuffer count: %u\n", surfaceDesc.dwBackBufferCount);
	if (surfaceFlags & DDSD_CAPS)
		write("capabilities: %X\n", surfaceDesc.ddsCaps.dwCaps);
	if (surfaceFlags & DDSD_CKDESTBLT)
		write("dest blt color key: %X %X\n", surfaceDesc.ddckCKDestBlt.dwColorSpaceLowValue, surfaceDesc.ddckCKDestBlt.dwColorSpaceHighValue);
	if (surfaceFlags & DDSD_CKDESTOVERLAY)
		write("dest overlay color key: %X %X\n", surfaceDesc.ddckCKDestOverlay.dwColorSpaceLowValue, surfaceDesc.ddckCKDestOverlay.dwColorSpaceHighValue);
	if (surfaceFlags & DDSD_CKSRCBLT)
		write("src blt color key: %X %X\n", surfaceDesc.ddckCKSrcBlt.dwColorSpaceLowValue, surfaceDesc.ddckCKSrcBlt.dwColorSpaceHighValue);
	if (surfaceFlags & DDSD_CKSRCOVERLAY)
		write("src overlay color key: %X %X\n", surfaceDesc.ddckCKSrcOverlay.dwColorSpaceLowValue, surfaceDesc.ddckCKSrcOverlay.dwColorSpaceHighValue);
	if (surfaceFlags & DDSD_LINEARSIZE)
		write("linear size: %u\n", surfaceDesc.dwLinearSize);
	if (surfaceFlags & DDSD_MIPMAPCOUNT)
		write("mipmap count: %u\n", surfaceDesc.dwMipMapCount);
	if (surfaceFlags & DDSD_PITCH)
		write("pitch: %ld\n", surfaceDesc.lPitch);
	if (surfaceFlags & DDSD_REFRESHRATE)
		write("refresh rate: %u\n", surfaceDesc.dwRefreshRate);
	if (surfaceFlags & DDSD_ZBUFFERBITDEPTH)
		write("z-buffer depth: %u\n", surfaceDesc.dwZBufferBitDepth);
	if (surfaceFlags & DDSD_PIXELFORMAT)
	{
		const DDPIXELFORMAT& pixelFormat = surfaceDesc.ddpfPixelFormat;
		write("pixel format:\n  flags: %X\n  RGB bit count: %u\n  Bit masks: %08X %08X %08X %08X",
			pixelFormat.dwFlags, pixelFormat.dwRGBBitCount, pixelFormat.dwRBitMask,
			pixelFormat.dwGBitMask, pixelFormat.dwBBitMask, pixelFormat.dwRGBAlphaBitMask);
	}

	return messageString;
}

BOOL drawFrame(HINSTANCE hInstance, HWND hWnd)
{
	// I'm just going to base this off of Flip_SystemTask()...
	static DWORD timePrev;

	DWORD timeNow = 0;
	while (true)
	{
		if (!handleEvents(hInstance))
			return FALSE;
		timeNow = timeGetTime();
		if (timeNow >= timePrev + 20)
			break;
		Sleep(1);
	}
	if (timeNow >= timePrev + 100)
		timePrev = timeNow;
	else
		timePrev += 20;
	
	// Get client area in screen coordinates
	RECT rect;
	GetClientRect(hWnd, &rect);
	MapWindowPoints(hWnd, NULL, reinterpret_cast<LPPOINT>(&rect), 2);

	frontbuffer->Blt(&rect, backbuffer, &windowRect, DDBLT_WAIT, NULL);

	restoreSurfaces();

	return TRUE;
}

class Roach
{
	int ani_no;
	int ani_wait;
	constexpr static RECT rect[4] = {
		{48*3, 0, 72*3, 24*3},
		{0, 0, 24*3, 24*3},
		{72*3, 0, 96*3, 24*3},
		{0, 0, 24*3, 24*3}
	};
public:
	Roach() : ani_no(0), ani_wait(0) {}
	void animate()
	{
		if (++ani_wait > 3)
		{
			ani_wait = 0;
			if (++ani_no > 3)
				ani_no = 0;
		}
	}

	void draw(int x, int y, LPDIRECTDRAWSURFACE surf) const
	{
		RECT rcSet = {x, y, x + 72, y + 72};
		RECT rcWork = rect[ani_no];
		
		backbuffer->Blt(&rcSet, surf, &rcWork, DDBLT_KEYSRC | DDBLT_WAIT, NULL);
	}
};

class Frog
{
	int ani_no;
	int ani_wait;
	int act_wait;
	int act_no;
	bool facingRight;
	bool jump;

	constexpr static RECT rcLeft[3] = {
		{0, 0, 32*3, 32*3},
		{32*3, 0, 64*3, 32*3},
		{64*3, 0, 96*3, 32*3},
	};
	constexpr static RECT rcRight[3] = {
		{0, 32*3, 32*3, 64*3},
		{32*3, 32*3, 64*3, 64*3},
		{64*3, 32*3, 96*3, 64*3},
	};
public:
	Frog(bool right = false) : ani_no(0), ani_wait(0), act_wait(0), act_no(0), facingRight(right), jump(false) {}
	void animate()
	{
		switch (act_no)
		{
		case 0:
		case 1:
			if (++act_wait > 10)
			{
				act_no = jump ? 3 : 2;
				act_wait = 0;
				ani_no = 0;
			}
			break;
		case 2:
			if (++ani_wait > 2)
			{
				ani_wait = 0;
				if (++ani_no > 1)
					ani_no = 0;
			}
			if (++act_wait > 18)
				act_no = 1;
			jump = true;
			break;
		case 3:
			ani_no = 2;
			if (++act_wait > 10)
			{
				act_no = 0;
				ani_no = 0;
				act_wait = 0;
				jump = false;
			}
		}
	}
	void draw(int x, int y, LPDIRECTDRAWSURFACE surf, bool overrideColorKey = false) const
	{
		RECT rcSet = {x, y, x + 96, y + 96};
		RECT rcWork = facingRight ? rcRight[ani_no] : rcLeft[ani_no];

		if (overrideColorKey)
		{
			DDBLTFX bltfx;
			memset(&bltfx, 0, sizeof bltfx); // Also sets color key to 0x00000000
			bltfx.dwSize = sizeof bltfx;
			backbuffer->Blt(&rcSet, surf, &rcWork, DDBLT_KEYSRCOVERRIDE | DDBLT_WAIT, &bltfx);
		}
		else
			backbuffer->Blt(&rcSet, surf, &rcWork, DDBLT_KEYSRC | DDBLT_WAIT, NULL);
	}
};

BOOL renderLoop(HINSTANCE hInstance, HWND hWnd)
{
	// Abort if not loaded
	if (backbuffer == NULL || roachSurface == NULL || roachSurfaceScaled == NULL)
		return FALSE;

	Roach leftFacingRoach, rightFacingRoach;
	Frog leftFacingFrog(false), rightFacingFrog(true);

	while (true)
	{
		// Animate roaches
		leftFacingRoach.animate();
		rightFacingRoach.animate();
		leftFacingFrog.animate();
		rightFacingFrog.animate();

		// Fill background with solid color
		DDBLTFX ddbltfx;
		memset(&ddbltfx, 0, sizeof ddbltfx);
		ddbltfx.dwSize = sizeof ddbltfx;
		ddbltfx.dwFillColor = 0x2D3A0C;
		backbuffer->Blt(&windowRect, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

		// Draw RoachClips
		rightFacingRoach.draw(44, 28, roachSurfaceScaled);
		leftFacingRoach.draw(204, 28, roachSurface);
		// Draw BigCroakers
		rightFacingFrog.draw(32, 128, frogSurface1);
		leftFacingFrog.draw(192, 128, frogSurface2, true);

		if (!drawFrame(hInstance, hWnd))
			return FALSE;
	}
}
