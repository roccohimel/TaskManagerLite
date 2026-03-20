#include <windows.h>
#include <pdh.h>
#include <string>

#pragma comment(lib, "pdh.lib")

PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;
double cpu = 0.0;

double getCPU() {
    PDH_FMT_COUNTERVALUE val;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &val);
    return val.doubleValue;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 1000, NULL);
            return 0;

        case WM_TIMER:
            cpu = getCPU();
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

            // Proper DPI-scaled Segoe UI (9pt)
            int fontSize = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);

            HFONT font = CreateFont(
                fontSize, 0, 0, 0,
                FW_NORMAL,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY,
                VARIABLE_PITCH,
                "Segoe UI"
            );

            HFONT oldFont = (HFONT)SelectObject(hdc, font);

            std::string text = "CPU Usage: " + std::to_string((int)cpu) + "%";

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));

            TextOutA(hdc, 12, 12, text.c_str(), text.length());

            SelectObject(hdc, oldFont);
            DeleteObject(font);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // 🔥 FIX BLUR (modern DPI awareness)
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const char CLASS_NAME[] = "TaskManagerLite";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Task Manager Lite",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 150,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    // CPU setup
    PdhOpenQuery(NULL, 0, &cpuQuery);
    PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    PdhCloseQuery(cpuQuery);
    return 0;
}