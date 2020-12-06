#include <Windows.h>
#include <cstddef>

namespace {

    //returns false if copying failed. Call ::GetLastError to get the error.
    bool copy_to_clipboard(HWND hwnd, const wchar_t* pText, std::size_t textLength) {
        const std::size_t textSizeInBytesWithTrailingNull = (textLength + 1) * sizeof(wchar_t);
        // copy the file name to the clipboard
        if (::HGLOBAL hGlobal = ::GlobalAlloc(GHND, textSizeInBytesWithTrailingNull)) {
            bool mustCallGlobalFree = true;
            if (auto mem = ::GlobalLock(hGlobal)) {
                ::CopyMemory(mem, pText, textSizeInBytesWithTrailingNull);

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
    }

    bool is_path_separator(wchar_t ch) {
        return (ch == L'\\' || ch == L'/');
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
        while (*foo) {
            foo++;
        }
        argsLength = foo - argsStart;

    } else {
        static wchar_t buffer[MAX_WIDE_PATH_LENGTH] = {};
        auto executablePathLength = ::GetModuleFileNameW(nullptr, buffer, MAX_WIDE_PATH_LENGTH);
        argsStart += executablePathLength;
    }

    // skip whitespace
    while (*argsStart == L' ') {
        argsStart += 1;
        argsLength -= 1;
    }

    //skip prefix " if any
    if (*argsStart == L'"') {
        argsStart += 1;
        argsLength -= 1;
    }

    const wchar_t* fileName = argsStart + argsLength;

    while (fileName > argsStart) {
        if (is_path_separator(*fileName)) {
            fileName++;
            break;
        }
        fileName--;
    }

    std::size_t nameLengthInChars = argsStart + argsLength - fileName;
    if (fileName[nameLengthInChars - 1] == L'"') { //remove trailing "
        nameLengthInChars -= 1;
    }

    copy_to_clipboard(nullptr, fileName, nameLengthInChars);

    ::ExitProcess(0);
}