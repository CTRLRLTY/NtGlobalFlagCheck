// NtGlobalFlagCheck.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "NtGlobalFlagCheck.h"

#define ID_REFRESH_BUTTON 1 // Identifier for the refresh button

// Forward declaration of the Window Procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND hButton; // Global handle for the button

// Custom function to check for debugger
bool isBeingDebugged() {
#define FLG_HEAP_ENABLE_TAIL_CHECK 0x10
#define FLG_HEAP_ENABLE_FREE_CHECK 0x20
#define FLG_HEAP_VALIDATE_PARAMETERS 0x40

    PPEB pPeb = (PPEB)__readgsqword(0x60);
    DWORD dwNtGlobalFlag = *(PDWORD)((PBYTE)pPeb + 0xBC);

    if (dwNtGlobalFlag & (FLG_HEAP_ENABLE_TAIL_CHECK
        | FLG_HEAP_ENABLE_FREE_CHECK
        | FLG_HEAP_VALIDATE_PARAMETERS))
    {
        return true;
    }

    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    // Register the window class.
    LPCWSTR CLASS_NAME = L"Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Debugger Check Window (NtGlobalFlag)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 600,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    // Create a refresh button
    hButton = CreateWindowEx(
        0, L"BUTTON", L"Refresh",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        0, 0, 100, 30, // Initial position and size of the button
        hwnd, (HMENU)ID_REFRESH_BUTTON, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Initial positioning of the button
    SendMessage(hwnd, WM_SIZE, 0, 0);

    // Run the message loop.
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_REFRESH_BUTTON) {
            // Redraw the window when the button is clicked
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Position the button below the text
        int buttonX = (rect.right - 100) / 2; // Center horizontally
        int buttonY = (rect.bottom / 2) + 20; // Below the text
        SetWindowPos(hButton, NULL, buttonX, buttonY, 100, 30, SWP_NOZORDER);

        // Invalidate the window to update text position
        InvalidateRect(hwnd, NULL, TRUE);
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Set up the text to display
        LPCWSTR text = isBeingDebugged() ? L"Debugger is present" : L"No debugger detected";
        int length = lstrlenW(text);

        // Calculate the position to center the text
        SIZE size;
        GetTextExtentPoint32W(hdc, text, length, &size);
        int x = (rect.right - size.cx) / 2;
        int y = (rect.bottom - size.cy) / 2 - 20; // Above the button

        // Draw the text
        TextOutW(hdc, x, y, text, length);

        // Set up the watermark text
        LPCWSTR watermarkText = L"Made By CTRLRLTY";
        SetTextColor(hdc, RGB(128, 128, 128)); // Grey text for the watermark
        SetBkMode(hdc, TRANSPARENT);

        // Draw the watermark at the bottom-right corner
        SIZE watermarkSize;
        GetTextExtentPoint32W(hdc, watermarkText, lstrlenW(watermarkText), &watermarkSize);
        TextOutW(hdc, rect.right - watermarkSize.cx - 10, rect.bottom - watermarkSize.cy - 10, watermarkText, lstrlenW(watermarkText));

        EndPaint(hwnd, &ps);
    }
    return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
