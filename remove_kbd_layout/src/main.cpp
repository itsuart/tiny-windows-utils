//#include <vector>
//#include <string>
//#include <format>
//#include <iostream>
#include <string_view>

#pragma comment(linker, "/nodefaultlib /entry:entry_point")

#define WIN32_LEAN_AND_MEAN
#define NO_MIN_MAX
#include <Windows.h>

// have to be in global namespace
void* operator new(std::size_t howMany) {
    return ::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, howMany);
}

void operator delete(void* what){
    ::HeapFree(::GetProcessHeap(), 0, what);
}


namespace {
    using namespace std::string_view_literals;

    void print(const std::wstring_view what) noexcept {
        ::WriteFile(::GetStdHandle(STD_OUTPUT_HANDLE), what.data(), what.size(), nullptr, nullptr);
    }

    std::uint32_t main_impl(){
        print(L"Hello, world!\n");
        return 0;
    }
}

void entry_point(){
    const auto retCode = main_impl();
    ::ExitProcess(retCode);
}

int wmain(int argc, wchar_t** argv) {

    //    const int nLayouts = ::GetKeyboardLayoutList(0, nullptr);
    //    std::vector<::HKL> currentLayouts{};
    //    currentLayouts.resize(nLayouts);
    /*
    const int ret = ::GetKeyboardLayoutList(currentLayouts.size(), currentLayouts.data());
    print(std::format(L"nLayouts = {}\n", nLayouts));

    std::wstring buffer;
    buffer.resize(KL_NAMELENGTH);

    for (auto hKeyboardLayout : currentLayouts) {
        ::ActivateKeyboardLayout(hKeyboardLayout, KLF_REORDER);
        if (::GetKeyboardLayoutNameW(buffer.data())) {
            if (L"00000409"sv == buffer.data()) {
                const bool unloaded = ::UnloadKeyboardLayout(hKeyboardLayout);
                print(std::format(
                                  L"hKeyboardLayout={}, name={}, unloaded={}\n"
                                  ,(void*)hKeyboardLayout, buffer.data(), unloaded));
            }
        }
    }
    */
}