#include <windows.h>

void main() {
    const wchar_t* commandLine = GetCommandLineW();
    const size_t length = (lstrlenW(commandLine) + 1); //for trailing zero
    if (IDYES == MessageBoxW(NULL, commandLine, L"Copy that to the Clipboard?", MB_YESNO)) {
        if (HGLOBAL hGlobal = GlobalAlloc(GHND, length * sizeof(wchar_t))) {
            if (auto mem = GlobalLock(hGlobal)) {
                lstrcpynW((wchar_t*)mem, commandLine, length);
                GlobalUnlock(hGlobal);

                if (OpenClipboard(NULL)) {
                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                    CloseClipboard();
                }
            }
            GlobalFree(hGlobal);
        }
    }
    ExitProcess(0);
}