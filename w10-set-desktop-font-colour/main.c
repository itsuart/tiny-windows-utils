#include <windows.h>

void exe_main(){
    const int colorType = COLOR_BACKGROUND;
    const COLORREF val = RGB(DESKTOP_COLOUR, DESKTOP_COLOUR, DESKTOP_COLOUR);
    SetSysColors(1, &colorType, &val);
    ExitProcess(0);
}
