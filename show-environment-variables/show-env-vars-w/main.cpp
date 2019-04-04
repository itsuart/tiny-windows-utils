#include <windows.h>
#include <string>

static wchar_t const* skipWholeEntry(wchar_t const * ptr) {
    while (*ptr != 0) {
        ptr += 1;
    }
    return ptr + 1;
}

static bool buildMessage(std::wstring& message) {
    wchar_t const * envVars = GetEnvironmentStringsW();
    if (envVars == nullptr) return false;

    auto currentPointer = envVars;
    while (*currentPointer != 0) {
        if (*currentPointer == L'=') {
            currentPointer = skipWholeEntry(currentPointer);
            continue;
        }
        //ok something real here!
        while (*currentPointer != 0) {
            message.push_back(*currentPointer);
            currentPointer += 1;
        }
        currentPointer += 1;
        message.append(L"\r\n");
    }

    return true;
}

static bool copyToClipboard(const std::wstring& message) {
    auto const memSize = (message.length() + 1) * sizeof(wchar_t);
    bool success = false;

    if (HGLOBAL hGlobal = GlobalAlloc(GHND, memSize)) { //for trailing zero
        if (auto mem = GlobalLock(hGlobal)) {
            std::memcpy(mem, message.c_str(), memSize);
            GlobalUnlock(hGlobal);

            if (OpenClipboard(NULL)) {
                success = nullptr != SetClipboardData(CF_UNICODETEXT, hGlobal);
                CloseClipboard();
            }
        }
        GlobalFree(hGlobal);
    }
    return success;
}

static bool wantToCopyToClipboard(const std::wstring& message) {
    return IDYES == MessageBoxW(
                                  NULL
                                , message.c_str(), L"Copy Environment Variables to Clipboard?"
                                , MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2
                               );
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    std::wstring message;
    
    if (!buildMessage(message)) return -1;

    if (0 == std::wcscmp(lpCmdLine, L"-silent") || wantToCopyToClipboard(message)){
        if (! copyToClipboard(message)) return -2;
    }

    return 0;
}
