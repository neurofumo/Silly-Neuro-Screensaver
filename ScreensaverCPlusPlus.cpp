// ScreensaverCPlusPlus.cpp : Defines the entry point for the application.
//

#include "resource.h"
#include "ScreensaverCPlusPlus.h"
#include <windows.h>
#include <gdiplus.h>
#include <time.h>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

// Global variables
HBITMAP hBitmap = nullptr; // Handle to the loaded image
int x = 100, y = 100;      // Current position of the image
int dx = 2, dy = 2;        // Movement direction
RECT screenRect;           // Screen dimensions

void LoadImageBitmap(HDC hdc) {
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (!hBitmap) {
        MessageBox(NULL, L"Failed to load bitmap from resources!", L"Error", MB_OK);
        exit(1);
    }
}


void DrawImage(HDC hdc) {
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hBitmap);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    // Clear the screen with a black background
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &screenRect, hBrush);
    DeleteObject(hBrush);

    // Draw the bitmap at the current position
    BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);

    DeleteDC(hdcMem);
}

void UpdatePosition() {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    x += dx;
    y += dy;

    // Bounce off edges
    if (x < 0 || x + bmp.bmWidth > screenRect.right) dx = -dx;
    if (y < 0 || y + bmp.bmHeight > screenRect.bottom) dy = -dy;
}
// Add global variables to track mouse movement
POINT initialMousePos;
bool isMouseMoved = false;

// Update the ScreenSaverProc function
LRESULT CALLBACK ScreenSaverProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        GetClientRect(hwnd, &screenRect);
        LoadImageBitmap(GetDC(hwnd));
        SetTimer(hwnd, 1, 10, NULL); // Set a timer for animation

        // Get the initial mouse position
        GetCursorPos(&initialMousePos);
        break;

    case WM_MOUSEMOVE: {
        // Check if the mouse has moved significantly
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        if (abs(currentMousePos.x - initialMousePos.x) > 5 ||
            abs(currentMousePos.y - initialMousePos.y) > 5) {
            isMouseMoved = true;
            PostQuitMessage(0); // Exit the screensaver
        }
        break;
    }

    case WM_TIMER:
        UpdatePosition();
        InvalidateRect(hwnd, NULL, FALSE); // Request a redraw
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawImage(hdc);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        DeleteObject(hBitmap);
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = ScreenSaverProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ScreenSaver";
    RegisterClass(&wc);

    // Create the fullscreen window
    HWND hwnd = CreateWindow(L"ScreenSaver", L"My Screensaver", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        GdiplusShutdown(gdiplusToken);
        return -1;
    }

    ShowWindow(hwnd, SW_SHOW);

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup GDI+
    GdiplusShutdown(gdiplusToken);
    return 0;
}
