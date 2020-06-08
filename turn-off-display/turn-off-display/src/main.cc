#include <Windows.h>

void entry_point() {
    ::PostMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
    ::ExitProcess(0);
}