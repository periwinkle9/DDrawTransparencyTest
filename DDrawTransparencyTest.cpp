// DDrawTransparencyTest.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DDrawTransparencyTest.h"
#include "draw.h"
#include "load.h"
#include <WindowsX.h>
#include <cstdio>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWindow;                                   // handle to the created window

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DDRAWTRANSPARENCYTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Initialize stuff
    if (!initDDraw(hWindow) || !loadSprites())
        return FALSE;
    
    MessageBox(hWindow, TEXT("Hi! If someone asked you to run this program, please screenshot the window "
        "along with the contents of Help > Technical Info and send it to them.\nThanks in advance :D"),
        TEXT("Hey!"), MB_OK);

    renderLoop(hInstance, hWindow);
    
    releaseDDraw();

    return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DDRAWTRANSPARENCYTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DDRAWTRANSPARENCYTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Same style as Doukutsu
   DWORD style = WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER | WS_DLGFRAME | WS_VISIBLE;
   RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
   if (!AdjustWindowRect(&rect, style, TRUE))
       return FALSE;

   int width = rect.right - rect.left;
   int height = rect.bottom - rect.top;

   hWindow = CreateWindowW(szWindowClass, szTitle, style,
      CW_USEDEFAULT, 0, width, height, nullptr, nullptr, hInstance, nullptr);

   if (!hWindow)
   {
      return FALSE;
   }

   ShowWindow(hWindow, nCmdShow);
   UpdateWindow(hWindow);

   return TRUE;
}

BOOL handleEvents(HINSTANCE hInstance)
{
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DDRAWTRANSPARENCYTEST));

    MSG msg;

    // Main message loop:
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    {
        if (!GetMessage(&msg, NULL, 0, 0))
            return FALSE;

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_HELP_INFO:
            {
                DDSURFACEDESC frontBufferDesc, frogSurfaceDesc;
                ZeroMemory(&frontBufferDesc, sizeof frontBufferDesc);
                ZeroMemory(&frogSurfaceDesc, sizeof frogSurfaceDesc);
                frontBufferDesc.dwSize = frogSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
                frontbuffer->GetSurfaceDesc(&frontBufferDesc);
                frogSurface2->GetSurfaceDesc(&frogSurfaceDesc);
                std::string messageText = "Frontbuffer description:\n" + getSurfaceDescString(frontBufferDesc)
                    + "\n\nFrog surface description:\n" + getSurfaceDescString(frogSurfaceDesc);
                MessageBoxA(hWnd, messageText.c_str(), "Surface descriptions", MB_OK | MB_ICONINFORMATION);
                break;
            }
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_LBUTTONDOWN:
        {
            int xpos = GET_X_LPARAM(lParam);
            int ypos = GET_Y_LPARAM(lParam);
            HDC hdc = GetDC(hWnd);
            if (hdc != NULL)
            {
                COLORREF pixColor = GetPixel(hdc, xpos, ypos);
                ReleaseDC(hWnd, hdc);

                char msg[50] = {};
                std::snprintf(msg, sizeof msg, "You clicked on the color (%u, %u, %u)", GetRValue(pixColor), GetGValue(pixColor), GetBValue(pixColor));
                MessageBoxA(NULL, msg, "Info", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
