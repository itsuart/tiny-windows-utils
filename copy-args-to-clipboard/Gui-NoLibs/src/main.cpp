#include <cstddef>

#include <Windows.h>

namespace {

    //returns false if copying failed. Call ::GetLastError to get the error.
    bool copy_to_clipboard(HWND hwnd, const wchar_t* pText, std::size_t textLength) {
        const int textSizeInBytesWithTrailingNull = (int)((textLength + 1) * sizeof(wchar_t));
        // copy the file name to the clipboard
        if (::HGLOBAL hGlobal = ::GlobalAlloc(GHND, textSizeInBytesWithTrailingNull)) {
            bool mustCallGlobalFree = true;
            if (auto mem = ::GlobalLock(hGlobal)) {
                ::lstrcpynA((char*)mem, (char*)pText, textSizeInBytesWithTrailingNull);

                {
                    const auto nLocksLeft = ::GlobalUnlock(hGlobal);
                    const auto errCode = ::GetLastError();
                    if (errCode != NO_ERROR) {
#ifdef _DEBUG
                        const auto errMessage = helpers::get_error_message_a(errCode);
#endif
                        return false;
                    }
                }

                if (::OpenClipboard(hwnd)) {
                    if (not ::EmptyClipboard()) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        //const auto errMessage = helpers::get_error_message_a(errCode);
#endif
                        ::OutputDebugStringA("::OpenClipboard call failed.");
                    }

                    if (NULL == ::SetClipboardData(CF_UNICODETEXT, hGlobal)) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        auto errMessage = helpers::get_error_message_a(errCode);
#endif
                        ::OutputDebugStringA("SetClipboardData call failed.");
                    }
                    else {
                        //System now owns the data
                        mustCallGlobalFree = false;
                    }

                    if (not ::CloseClipboard()) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        //const auto errMessage = helpers::get_error_message_a(errCode);
#endif
                        ::OutputDebugStringA("::CloseClipboard call failed.");
                    }

                }
            }

            if (mustCallGlobalFree) {
                ::GlobalFree(hGlobal);
                return false;
            }
            return true;
        }

        return false;
    }

    constexpr int MAX_WIDE_PATH_LENGTH = 32767;
}


void entry_point() {
    const wchar_t* commandLine = ::GetCommandLineW();
    const wchar_t* argsStart = commandLine;
    size_t argsLength = 0;
    if (commandLine[0] == L'"') {
        //path to executable is quoted, let's 'unqoute' it

        bool maybeEscapeThis = false;
        bool inQuote = false;
        bool keepWorking = true;
        while (*argsStart && keepWorking) {
            if (maybeEscapeThis) {
                //we don't care about actual value, but it certainly not starting/finishisg quote mark, so
                argsStart += 1;
                maybeEscapeThis = false;
                continue;
            }
            const wchar_t currentChar = *argsStart;
            switch (currentChar) {
                case L'"': {
                    if (inQuote) {
                        //its a finishing quotemark
                        argsStart += 1;
                        keepWorking = false;
                        inQuote = false;
                    } else {
                        //its starting quotemark
                        inQuote = true;
                        argsStart += 1;
                    }
                } break;

                case L'\\': {
                    maybeEscapeThis = true;
                    argsStart += 1;
                } break;

                default: {
                    //move along
                    argsStart += 1;
                }
            }
        }
        //count leftover space
        auto foo = argsStart;
        while (*foo++);
        argsLength = foo - argsStart;

    } else {
        static wchar_t buffer[MAX_WIDE_PATH_LENGTH] = { 0 };
        auto executablePathLength = ::GetModuleFileNameW(NULL, buffer, MAX_WIDE_PATH_LENGTH);
        argsStart += executablePathLength;
    }

    // skip whitespace
    while (*argsStart == L' ') {
        argsStart += 1;
        argsLength -= 1;
    }

    copy_to_clipboard(nullptr, argsStart, argsLength);

    if (HGLOBAL hGlobal = ::GlobalAlloc(GHND, argsLength * sizeof(wchar_t))) {
        if (auto mem = ::GlobalLock(hGlobal)) {

            ::lstrcpyW((wchar_t*)mem, argsStart);
            ::GlobalUnlock(hGlobal);

            if (::OpenClipboard(NULL)) {
                ::SetClipboardData(CF_UNICODETEXT, hGlobal);
                ::CloseClipboard();
            }
        }
        ::GlobalFree(hGlobal);
    }
    ::ExitProcess(0);
}