#include <Windows.h>
#include <wtsapi32.h>

void entry_point(){
    WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, false);
    ExitProcess(0);
}
