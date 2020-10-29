#include <windows.h>
#include <iostream>
#include <memory>

#include "context.hpp"

struct GrBitmap {
    HWND owner_window;   // Owner window, if any
    HDC device;          // Device context of the bitmap
    HBITMAP bitmap;      // Handle to the bitmap
    long _cached_width;  // Cached width of the bitmap
    long _cached_height; // Cached height of the bitmap
};

// Library

void GetBitmapSize(const GrBitmap* bitmap, long& width, long& height) {
    width = bitmap->_cached_width;
    height = bitmap->_cached_height;
}

void GrClearBuffer(GrBitmap* bmap, GrColor color) {
    RECT rect {0, 0, bmap->_cached_width, bmap->_cached_height};
    HBRUSH brush = CreateSolidBrush(color & 0x00'FFFFFFU);
    FillRect(bmap->device, &rect, brush);
    DeleteObject(brush);
}

void GrDrawLine(GrBitmap* bmap, GrColor color, GrPos start, GrPos end) {
    HPEN pen = CreatePen(PS_SOLID, 1, color & 0x00'FFFFFFU);
    HPEN pen_old = (HPEN)SelectObject(bmap->device, pen);
    MoveToEx(bmap->device, start.x, start.y, nullptr);
    LineTo(bmap->device, end.x, end.y);
    SelectObject(bmap->device, pen_old);
    DeleteObject(pen);
}

// Backend

GrContext gAppContext {nullptr, nullptr};

static void InitBitmap(GrBitmap& bitmap, HWND window, long width, long height) {
    HDC hwdc = GetDC(window);
    HDC hdc = CreateCompatibleDC(hwdc);
    HBITMAP bmap = CreateCompatibleBitmap(hwdc, width, height);
    SelectObject(hdc, bmap);

    bitmap.bitmap = bmap;
    bitmap.device = hdc;
    bitmap.owner_window = window;
    bitmap._cached_width = width;
    bitmap._cached_height = height;
}

static void DestroyBitmap(GrBitmap& bitmap) {
    DeleteDC(bitmap.device);
    DeleteObject(bitmap.bitmap);

    // Fill with zeros
    std::memset(&bitmap, 0, sizeof(GrBitmap));
}

static void HandleResize(HWND window, long width, long height) {
    DestroyBitmap(*gAppContext.framebuffer);
    InitBitmap(*gAppContext.framebuffer, window, width, height);
}

static LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
        {
            // Render first
            Update(gAppContext, GrMessage::Render);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(window, &ps);
            long w, h;
            GetBitmapSize(gAppContext.framebuffer, w, h);
            BitBlt(hdc, 0, 0, w, h, gAppContext.framebuffer->device, 0, 0, SRCCOPY);
            EndPaint(window, &ps);
            break;
        }

        case WM_SIZE:
        {
            RECT crect;
            GetClientRect(window, &crect);
            HandleResize(window, crect.right, crect.bottom);
            InvalidateRect(window, nullptr, true);
            break;
        }
        
        case WM_CLOSE:
            DestroyWindow(window);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        //case WM_NCCREATE:
        //    break;

        default:
        return DefWindowProc(window, msg, wParam, lParam);
    }
    return 0;
}

static void DoCreateClass(HINSTANCE hProg) {
    WNDCLASSEXA wc {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = CS_OWNDC,
        .lpfnWndProc = &WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hProg,
        .hIcon = nullptr,
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW+1),
        .lpszMenuName = nullptr,
        .lpszClassName = "grapher_window",
        .hIconSm = nullptr
    };

    if (RegisterClassEx(&wc) == 0)
        ExitProcess(1);
}

int WINAPI WinMain(HINSTANCE hProg, [[maybe_unused]]HINSTANCE hPrevInstance, [[maybe_unused]]char* cmd_line, int show_mode) {
    DoCreateClass(hProg);
    HWND window = CreateWindowEx(0, "grapher_window", "GraphEx", WS_TILEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, 0, 1280, 720, nullptr, nullptr, hProg, nullptr);
    if (window == nullptr) {
        std::cerr << "No window" << std::endl;
        return 1;
    }

    // Create window bitmap
    GrBitmap window_bitmap;
    RECT crect;
    GetClientRect(window, &crect);
    InitBitmap(window_bitmap, window, crect.right, crect.bottom);

    gAppContext.framebuffer = &window_bitmap;

    ShowWindow(window, show_mode);

    Update(gAppContext, GrMessage::Init);
    MSG wnd_msg;
    while (GetMessage(&wnd_msg, nullptr, 0, 0)) {
        TranslateMessage(&wnd_msg);
        DispatchMessage(&wnd_msg);
    }

    // Cleanup
    DestroyBitmap(window_bitmap);
    return 0;
}
