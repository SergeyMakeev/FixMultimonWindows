#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>

static TCHAR title[2048];

static std::vector<HWND> windows;

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
{
    windows.emplace_back(hWnd);
    return TRUE;
}

int main()
{
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    if (hMonitor == nullptr)
    {
        printf("Can't get monitor from cursor position\n");
        return -1;
    }

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    BOOL res = GetMonitorInfoW(hMonitor, &mi);
    if (res == FALSE)
    {
        printf("Can't get monitor info\n");
        return -2;
    }

    const RECT& rect = mi.rcWork;
    printf("Active monitor rect: %d,%d to %d,%d\n", rect.left, rect.top, rect.right, rect.bottom);

    if (!EnumWindows(enumWindowCallback, NULL))
    {
        printf("Enum windows failed\n");
        return -3;
    }

    printf("%d windows found\n", (int)windows.size());
    int offset = 0;
    for(size_t i = 0; i < windows.size(); i++)
    {
        HWND hCurWnd = windows[i];
        if (!hCurWnd)
        {
            continue;
        }
        memset(&title[0], 0, sizeof(title));
        int num = GetWindowText(hCurWnd, title, 2047);
        if (num == 0 || title[0] == '\0')
        {
            continue;
        }

        LONG style = GetWindowLong(hCurWnd, GWL_STYLE);
        if ((style & WS_VISIBLE) == 0)
        {
            continue;
        }

        WINDOWPLACEMENT windowPlacement;
        windowPlacement.length = sizeof(windowPlacement);
        if (!GetWindowPlacement(hCurWnd, &windowPlacement))
        {
            continue;
        }

        if (windowPlacement.showCmd == SW_HIDE)
        {
            continue;
        }


        RECT wndRect;
        if (!GetClientRect(hCurWnd, &wndRect))
        {
            continue;
        }

        int width = wndRect.right - wndRect.left;
        int height = wndRect.bottom - wndRect.top;

        if (width == 0 || height == 0)
        {
            continue;
        }

        printf("%S, %X\n", title, style);
        printf("  - Window rect: %d,%d to %d,%d\n", wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

/*
        RECT clipRect;
        if (IntersectRect(&clipRect, &wndRect, &rect))
        {
            if (clipRect.right > clipRect.left || clipRect.bottom > clipRect.top)
            {
                // window already visible
                continue;
            }
        }
*/


        if ((style & WS_POPUP) != 0)
        {
            ShowWindow(hCurWnd, SW_RESTORE);
        }

        SetWindowPos(hCurWnd, nullptr, rect.left + offset, rect.top + offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        offset += 2;
    }

    return 0;
}

