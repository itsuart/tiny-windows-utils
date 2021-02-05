#include <Windows.h>
#include <string>
#include <memory>
#include <cassert>
#include <type_traits>

namespace {
    std::unique_ptr<std::FILE, decltype(&std::fclose)> open_file(const wchar_t* fileName, const wchar_t* mode) {
        assert(fileName);
        assert(mode);
        std::unique_ptr<FILE, decltype(&std::fclose)> file(_wfopen(fileName, mode), &std::fclose);
        return file;
    }

    std::unique_ptr<std::FILE, decltype(&std::fclose)> open_file_read(const wchar_t* fileName) {
        return open_file(fileName, L"rb");
    }

    std::unique_ptr<std::FILE, decltype(&std::fclose)> open_file_write(const wchar_t* fileName) {
        return open_file(fileName, L"wb");
    }


    void write_to_file(FILE* file, const char* data, std::size_t dataSize) {
        while (dataSize > 0) {
            dataSize -= std::fwrite(data, 1, dataSize, file);
        }
    }

    template<typename T=char, typename = std::enable_if_t<1 == sizeof(T)> >
    std::basic_string<T> read_whole_file(FILE* f) {
        constexpr std::size_t BUFFER_SIZE = 4 * 1024;
        T buffer[BUFFER_SIZE];

        std::basic_string<T> content;
        content.reserve(BUFFER_SIZE);

        while (true) {
            const std::size_t bytesRead = std::fread(buffer, 1, sizeof(buffer), f);
            if (bytesRead == 0) {
                break;
            }
            content.append(buffer, bytesRead);
        }

        return content;
    }


    std::unique_ptr<char, decltype(LocalFree)*> get_error_message_a(DWORD errorCode) noexcept {
        char* message = nullptr;
        ::FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr, errorCode, 0, (char*)&message, 1, nullptr);
        return std::unique_ptr<char, decltype(LocalFree)*>(message, LocalFree);
    }

    std::unique_ptr<wchar_t, decltype(LocalFree)*> get_error_message_w(DWORD errorCode) noexcept {
        wchar_t* message = nullptr;
        ::FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr, errorCode, 0, (wchar_t*)&message, 1, nullptr);
        return std::unique_ptr<wchar_t, decltype(LocalFree)*>(message, LocalFree);
    }

    //returns false if copying failed. Call ::GetLastError to get the error.
    template<typename T, typename = std::enable_if_t<(sizeof(T) < 3)> >
    bool copy_to_clipboard(HWND hwnd, std::basic_string_view<T> text) {
        // copy the file name to the clipboard
        if (::HGLOBAL hGlobal = ::GlobalAlloc(GHND, (text.size() + 1) * sizeof(T))) {
            bool mustCallGlobalFree = true;
            if (auto mem = ::GlobalLock(hGlobal)) {
                text.copy((T*)mem, text.size());
                {
                    const auto nLocksLeft = ::GlobalUnlock(hGlobal);
                    if (nLocksLeft != 0) {
                        assert(false and "should not be possible");
                    }
                    const auto errCode = ::GetLastError();
                    if (errCode != NO_ERROR) {
#ifdef _DEBUG
                        const auto errMessage = get_error_message_a(errCode);
#endif
                        return false;
                    }
                }

                if (::OpenClipboard(hwnd)) {
                    if (not ::EmptyClipboard()) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        const auto errMessage = get_error_message_a(errCode);
#endif
                        assert(false);
                    }

                    if (NULL == ::SetClipboardData((sizeof(T) == 1) ? CF_TEXT : CF_UNICODETEXT, hGlobal)) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        auto errMessage = get_error_message_a(errCode);
#endif
                        assert(false);
                    }
                    else {
                        //System now owns the data
                        mustCallGlobalFree = false;
                    }

                    if (not ::CloseClipboard()) {
#ifdef _DEBUG
                        const auto errCode = ::GetLastError();
                        const auto errMessage = get_error_message_a(errCode);
#endif
                        assert(false);
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

    void show_error_message(const wchar_t* message) {
        ::MessageBoxW(nullptr, message, L"Error", MB_OK | MB_ICONERROR);
    }

    void show_info_message(const wchar_t* message) {
        ::MessageBoxW(nullptr, message, L"Information", MB_OK | MB_ICONINFORMATION);
    }

}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    int nArgs{ 0 };
    wchar_t** args = ::CommandLineToArgvW(::GetCommandLineW(), &nArgs);
    if (nArgs != 2) {
        show_error_message(L"Please provide path to the file as a command line argument");
        return -1;
    }

    const wchar_t* pathToTheFile = args[1];
    std::wstring safeLongPath(LR"(\\?\)");
    safeLongPath.append(pathToTheFile);

    auto maybeFile = open_file_read(safeLongPath.c_str());
    if (not maybeFile) {
        show_error_message(std::wstring(L"Failed to open ").append(safeLongPath).c_str());
        return -1;
    }

    const std::string contents = read_whole_file(maybeFile.get());

    //TODO: speed up
    for (char c : contents){
        const bool isSpace = isspace(c);
        const bool isPrint = isprint(c);
        if (not (isSpace || isPrint)){
            show_error_message(L"File contains non-supported characters.");
            return -1;
        }
    }

    if (copy_to_clipboard<char>(nullptr, contents)){
        show_info_message(std::wstring(L"Copied contents of ").append(safeLongPath).c_str());
        return 0;
    }
    else {
        show_error_message(L"Failed to copy the file content into the clipboard.");
        return -1;
    }

}