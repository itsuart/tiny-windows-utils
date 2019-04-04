#define UNICODE
#include <windows.h>
#include <stdbool.h>

static bool is_button_down(int virtual_key){
    const short BTN_DOWN = 0x8000;
    return GetKeyState (virtual_key) & BTN_DOWN;
}

HHOOK hHook = 0;

static void display_last_error(){
    DWORD last_error = GetLastError();
    unsigned short* reason = NULL;

    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, last_error, 0, (LPWSTR)&reason, 1, NULL);
    MessageBoxW(NULL, reason, NULL, MB_OK);
    HeapFree(GetProcessHeap(), 0, reason);
}

__declspec(dllexport) LRESULT CALLBACK hook_proc( int code, WPARAM wParam, LPARAM lParam ){
    if (wParam == VK_BACK && (lParam & (1 << 31)) && is_button_down(VK_CONTROL) && is_button_down(VK_MENU)){
        HWND foreground_window = GetForegroundWindow();

        if (foreground_window != NULL){
            DWORD proc_id = 0;
            GetWindowThreadProcessId(foreground_window, &proc_id);
            HANDLE h_process = OpenProcess(PROCESS_TERMINATE, FALSE, proc_id);
            if (NULL == h_process){
                display_last_error();
            } else {
                if (!TerminateProcess(h_process, 100)){
                    display_last_error();
                }
                CloseHandle(h_process);
            }
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}

void dll_main(){/*do nothing*/}

static void do_work(){
    HMODULE dll = LoadLibraryA("hook.dll");
    if (dll == NULL){
        display_last_error();
        return;
    }

    void* proc = GetProcAddress(dll, "hook_proc");
    if  (proc == NULL){
        display_last_error();
        return;
    }

    hHook = SetWindowsHookExW(WH_KEYBOARD, proc, dll, 0);
    if (hHook == NULL){
        display_last_error();
        return;
    }

    MSG msg;
    BOOL bRet;
    while((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if(bRet == -1) {
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    UnhookWindowsHookEx(hHook);
}

void exe_main(){
    do_work();
    ExitProcess(0);
}
