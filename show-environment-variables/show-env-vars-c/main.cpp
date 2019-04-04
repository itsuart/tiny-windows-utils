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

int main() {
    std::wstring message;
    buildMessage(message);
    DWORD dontcare;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), message.c_str(), message.length(), &dontcare, NULL);
}