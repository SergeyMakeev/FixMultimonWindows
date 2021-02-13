#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

static TCHAR title[2048];

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

    int offset = 0;
    HWND hCurWnd = NULL;
    for(;;)
    {
        hCurWnd = FindWindowEx(NULL, hCurWnd, NULL, NULL);
        if (!hCurWnd)
        {
            break;
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

        RECT clipRect;
        if (IntersectRect(&clipRect, &wndRect, &rect))
        {
            if (clipRect.right > clipRect.left || clipRect.bottom > clipRect.top)
            {
                // window already visible
                continue;
            }
        }

        int width = wndRect.right - wndRect.left;
        int height = wndRect.bottom - wndRect.top;

        if (width == 0 || height == 0)
        {
            continue;
        }

        printf("%S, %X\n", title, style);
        printf("  - Window rect: %d,%d to %d,%d\n", wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

        if ((style & WS_POPUP) != 0)
        {
            ShowWindow(hCurWnd, SW_RESTORE);
        }

        SetWindowPos(hCurWnd, nullptr, rect.left + offset, rect.top + offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        offset += 2;
    }

    return 0;
}

