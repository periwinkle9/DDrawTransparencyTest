#include "load.h"
#include "draw.h"
#include "resource.h"

#define STBI_ONLY_BMP
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

BOOL loadRoachClipLeft()
{
	// Load bitmap
	HRSRC resourceInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_ROACHCLIPLEFT), RT_RCDATA);
	if (resourceInfo == NULL)
		return FALSE;
	HGLOBAL resource = LoadResource(NULL, resourceInfo);
	if (resource == NULL)
		return FALSE;
	unsigned char* resourceBytes = reinterpret_cast<unsigned char*>(LockResource(resource));
	if (resourceBytes == NULL)
		return FALSE;

	int bmpWidth, bmpHeight;
	unsigned char* bmpData = stbi_load_from_memory(resourceBytes, SizeofResource(NULL, resourceInfo), &bmpWidth, &bmpHeight, nullptr, 4);
	if (bmpData == nullptr)
		return FALSE;

	// Create surface
	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bmpWidth;
	ddsd.dwHeight = bmpHeight;
	if (lpDD->CreateSurface(&ddsd, &roachSurface, NULL) != DD_OK)
		return FALSE;

	// Lock surface in preparation for drawing
	memset(&ddsd, 0, sizeof ddsd); // Not sure if it's necessary to clear this out, but let's just do it to be safe
	ddsd.dwSize = sizeof ddsd; // Let's also set this just to be safe

	// I'm leaking so much memory if any of this stuff fails, but I'm too lazy to fix that (the OS will clean up for me anyways :P )
	if (roachSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL) != DD_OK)
		return FALSE;
	if ((ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) == 0 || ddsd.ddpfPixelFormat.dwRGBBitCount != 32)
	{
		MessageBox(NULL, TEXT("What kind of computer do you have?!"), TEXT("Your computer is weird!"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (ddsd.lPitch < bmpWidth * 4)
	{
		roachSurface->Unlock(NULL);
		return FALSE;
	}

	auto getShiftFromMask = [](unsigned mask) {
		switch (mask)
		{
		case 0x000000FFu:
			return 0;
		case 0x0000FF00u:
			return 8;
		case 0x00FF0000u:
			return 16;
		case 0xFF000000u:
			return 24;
		default:
			return -1;
		}
	};
	const int RShift = getShiftFromMask(ddsd.ddpfPixelFormat.dwRBitMask);
	const int GShift = getShiftFromMask(ddsd.ddpfPixelFormat.dwGBitMask);
	const int BShift = getShiftFromMask(ddsd.ddpfPixelFormat.dwBBitMask);
	if (RShift == -1 || GShift == -1 || BShift == -1)
	{
		// idk what's even happening if this is true
		roachSurface->Unlock(NULL);
		return FALSE;
	}

	// Draw bitmap pixels to surface (in the slowest way possible, lol)
	unsigned int* bmpPixelData = reinterpret_cast<unsigned int*>(bmpData);
	unsigned int* surfacePixel = reinterpret_cast<unsigned int*>(ddsd.lpSurface);
	for (int y = 0; y < bmpHeight; ++y)
	{
		for (int x = 0; x < bmpWidth; ++x)
		{
			unsigned bmpPixel = bmpPixelData[x + y * bmpWidth];
			unsigned red = (bmpPixel & 0xFF);
			unsigned green = (bmpPixel & 0xFF00) >> 8;
			unsigned blue = (bmpPixel & 0xFF0000) >> 16;
			unsigned convertedPixel = (red << RShift) | (green << GShift) | (blue << BShift);
			surfacePixel[x + y * (ddsd.lPitch >> 2)] = convertedPixel;
		}
	}

	roachSurface->Unlock(NULL);
	stbi_image_free(bmpData);

	DDCOLORKEY colorKey;
	colorKey.dwColorSpaceLowValue = 0;
	colorKey.dwColorSpaceHighValue = 0;
	if (roachSurface->SetColorKey(DDCKEY_SRCBLT, &colorKey) != DD_OK)
		return FALSE;

	return TRUE;
}

BOOL loadRoachClipRight()
{
	// Mimicking how Doukutsu loads bitmaps
	HANDLE handle = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ROACHCLIPRIGHT), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (handle == NULL)
		return FALSE;

	BITMAP bitmap;
	GetObject(handle, sizeof(BITMAP), &bitmap);

	// Create surface
	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bitmap.bmWidth * 3;
	ddsd.dwHeight = bitmap.bmHeight * 3;
	if (lpDD->CreateSurface(&ddsd, &roachSurfaceScaled, NULL) != DD_OK)
		return FALSE;

	// Copy bitmap onto surface
	HDC hdc = CreateCompatibleDC(NULL);
	HGDIOBJ hgdiobj = SelectObject(hdc, handle);
	HDC hdc2;
	if (roachSurfaceScaled->GetDC(&hdc2) != DD_OK)
		return FALSE;
	if (!StretchBlt(hdc2, 0, 0, bitmap.bmWidth * 3, bitmap.bmHeight * 3, hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY))
		return FALSE;
	roachSurfaceScaled->ReleaseDC(hdc2);
	SelectObject(hdc, hgdiobj);
	DeleteDC(hdc);

	DDCOLORKEY colorKey;
	colorKey.dwColorSpaceLowValue = 0;
	colorKey.dwColorSpaceHighValue = 0;
	if (roachSurfaceScaled->SetColorKey(DDCKEY_SRCBLT, &colorKey) != DD_OK)
		return FALSE;

	DeleteObject(handle);

	return TRUE;
}

BOOL loadFrog1()
{
	// Using BitBlt
	HANDLE handle = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BIGCROAKER2), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (handle == NULL)
		return FALSE;

	BITMAP bitmap;
	GetObject(handle, sizeof(BITMAP), &bitmap);

	// Create surface
	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bitmap.bmWidth;
	ddsd.dwHeight = bitmap.bmHeight;
	if (lpDD->CreateSurface(&ddsd, &frogSurface1, NULL) != DD_OK)
		return FALSE;

	// Copy bitmap onto surface
	// Let's try something slightly different here
	HDC hdc2;
	if (frogSurface1->GetDC(&hdc2) != DD_OK)
		return FALSE;
	HDC hdc = CreateCompatibleDC(hdc2);
	HGDIOBJ hgdiobj = SelectObject(hdc, handle);
	
	if (!BitBlt(hdc2, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdc, 0, 0, SRCCOPY))
		return FALSE;
	frogSurface1->ReleaseDC(hdc2);
	SelectObject(hdc, hgdiobj);
	DeleteDC(hdc);

	DDCOLORKEY colorKey;
	colorKey.dwColorSpaceLowValue = 0x01010101u; // Hopefully this should work regardless of color format
	colorKey.dwColorSpaceHighValue = 0x01010101u;
	if (frogSurface1->SetColorKey(DDCKEY_SRCBLT, &colorKey) != DD_OK)
		return FALSE;

	DeleteObject(handle);

	return TRUE;
}

BOOL loadFrog2()
{
	// Using TransparentBlt
	HANDLE handle = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BIGCROAKER), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (handle == NULL)
		return FALSE;

	BITMAP bitmap;
	GetObject(handle, sizeof(BITMAP), &bitmap);

	// Create surface
	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof ddsd);
	ddsd.dwSize = sizeof ddsd;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwWidth = bitmap.bmWidth;
	ddsd.dwHeight = bitmap.bmHeight;
	if (lpDD->CreateSurface(&ddsd, &frogSurface2, NULL) != DD_OK)
		return FALSE;

	// Black out the surface
	DDBLTFX ddbltfx;
	memset(&ddbltfx, 0, sizeof ddbltfx);
	ddbltfx.dwSize = sizeof ddbltfx;
	ddbltfx.dwFillColor = 0x000000;
	frogSurface2->Blt(&windowRect, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	// Copy bitmap onto surface
	HDC hdc = CreateCompatibleDC(NULL);
	HGDIOBJ hgdiobj = SelectObject(hdc, handle);
	HDC hdc2;
	if (frogSurface2->GetDC(&hdc2) != DD_OK)
		return FALSE;

	if (!TransparentBlt(hdc2, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0))
		return FALSE;
	frogSurface2->ReleaseDC(hdc2);
	SelectObject(hdc, hgdiobj);
	DeleteDC(hdc);

	DDCOLORKEY colorKey;
	colorKey.dwColorSpaceLowValue = 0;
	colorKey.dwColorSpaceHighValue = 0;
	if (frogSurface2->SetColorKey(DDCKEY_SRCBLT, &colorKey) != DD_OK)
		return FALSE;

	DeleteObject(handle);

	return TRUE;
}

BOOL loadSprites()
{
	return loadRoachClipLeft() && loadRoachClipRight() && loadFrog1() && loadFrog2();
}
