#include <Windows.h>

static constexpr int MAX_WIDE_PATH_LENGTH = 32767;
void entry_point(){
    const wchar_t* commandLine = GetCommandLineW();
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

    // skip separating whitespace
    while (*argsStart == L' ') {
        argsStart += 1;
    }


    ::ShellExecuteW(nullptr, L"open", argsStart, nullptr, nullptr, SW_HIDE);
    ::ExitProcess(0);
}
