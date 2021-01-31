#include <Windows.h>

namespace {
    template <typename T>
    T abs(T val) {
        return (val < 0) ? (-val) : val;
    }

    struct Rect : RECT {
        LONG width() const {
            return abs(right - left);
        }

        void set_width(decltype (left) value) {
            right = left + value;
        }

        LONG height() const {
            return abs(bottom - top);
        }

        void set_height(decltype(top) value) {
            bottom = top + value;
        }

        void apply_dx(LONG delta) {
            left += delta;
            right += delta;
        }

        void apply_dy(LONG delta) {
            top += delta;
            bottom += delta;
        }
    };


    void set_window_pos(HWND wnd, const RECT& rect) {
        ::SetWindowPos(
            wnd, NULL,
            rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }

    int __stdcall ProcessToplevelWindow(HWND wnd, LPARAM param) {
        Rect windowRect{};
        if (::GetWindowRect(wnd, &windowRect)) {

            if (::HMONITOR hMonitor = ::MonitorFromWindow(wnd, MONITOR_DEFAULTTONULL)) {
                MONITORINFO mi;
                mi.cbSize = sizeof(mi);

                if (::GetMonitorInfoW(hMonitor, &mi)) {
                    Rect monitorRect{};
                    ((RECT&)monitorRect) = mi.rcWork;

                    //first we deal with the size of the window
                    if (windowRect.width() > monitorRect.width()){
                        windowRect.set_width(monitorRect.width());
                    }
                    if (windowRect.height() > monitorRect.height()){
                        windowRect.set_height(monitorRect.height());
                    }

                    // Now check which part of the window is extruding (if any) and snap it
                    // to the respective edge of the monitor.

                    if (windowRect.left < monitorRect.left){
                        windowRect.apply_dx(monitorRect.left - windowRect.left);
                    } else if (windowRect.right > monitorRect.right){
                        windowRect.apply_dx(monitorRect.right - windowRect.right);
                    }

                    if (windowRect.top < monitorRect.top){
                        windowRect.apply_dy(monitorRect.top - windowRect.top);
                    } else if (windowRect.bottom < monitorRect.bottom){
                        windowRect.apply_dy(monitorRect.bottom - windowRect.bottom);
                    }

                    set_window_pos(wnd, windowRect);
                }
            }

        }
        return TRUE;
    }
}

void entry_point() {
    ::EnumWindows(&ProcessToplevelWindow, 0);
    ::ExitProcess(0);
}
