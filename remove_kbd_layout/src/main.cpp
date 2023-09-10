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


namespace ndl{
    template<typename T>
    T* allocate_memory(std::size_t howManyItems){
        return static_cast<T*>(
                               ::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, howManyItems * sizeof(T))
                               );
    }

    void free_memory(void* memory){
        ::HeapFree(::GetProcessHeap(), 0, memory);
    }

    template<typename T>
    class Dynamic_Array final{
    public:
        Dynamic_Array()
        : m_data(nullptr)
        , m_size(0)
        , m_capacity(0)
        {
        }

        ~Dynamic_Array() noexcept {
            if (m_data == nullptr){
                return;
            }

            for (std::size_t i = 0; i < m_size; ++i){
                m_data[i]->~T();
            }
            free_memory(m_data);

            m_data = nullptr;
            m_size = 0;
            m_capacity = 0;
        }

        //copying and moving disabled because I don't need them
        Dynamic_Array(const Dynamic_Array&) = delete;
        Dynamic_Array& operator=(const Dynamic_Array&) = delete;

        Dynamic_Array(Dynamic_Array&&) = delete;
        Dynamic_Array& operator=(Dynamic_Array&&) = delete;

        std::size_t push_back(const T& item){
            if (m_size == m_capacity){
                resize(m_capacity * 3 / 2);
            }

            std::size_t indexOfNewItem = m_size;
            m_data[m_size++]->T(item);
            return indexOfNewItem;
        }

        T& at(std::size_t index) const {
            if (index >= m_size){
                return *(T*)nullptr;
            }

            return m_data[index];
        }

        std::size_t size() const noexcept {
            return m_size;
        }

        std::size_t capacity() const noexcept {
            return m_capacity;
        }

    private:
        void resize(std::size_t nextCapacity){
            T* nextData = allocate_memory<T>(nextCapacity);
            for (std::size_t i = 0; i < m_size; ++i){
                T* pOldItem = m_data[i];
                T* pNewItem = nextData[i];
                pNewItem->T((T&&)*pOldItem);
            }

            for (std::size_t i = 0; i < m_size; ++i){
                m_data[i]->~T();
            }

            m_data = nextData;
            m_capacity = nextCapacity;
        }

    private:
        T* m_data;
        std::size_t m_size;
        std::size_t m_capacity;
    };

    template<typename T>
    class Static_Array final{
    public:
        explicit Static_Array(std::size_t size)
            : m_data(allocate_memory<T>(size))
            , m_size(size)
        {}

        ~Static_Array() noexcept {
            for (std::size_t i = 0; i < m_size; ++i){
                ((T*)m_data[i])->~T();
            }
            free_memory(m_data);

            m_data = nullptr;
            m_size = 0;
        }

        Static_Array(const Static_Array&) = delete;
        Static_Array(Static_Array&&) = delete;

        Static_Array& operator=(const Static_Array&) = delete;
        Static_Array& operator=(Static_Array&&) = delete;

        const std::size_t size() const noexcept {
            return m_size;
        }

        T& at(std::size_t index) {
            if (index >= m_size) {
                return *((T*) nullptr);
            }

            return *m_data[index];
        }

        const T& at(std::size_t index) const {
            if (index >= m_size) {
                return *((T*) nullptr);
            }

            return *m_data[index];
        }


        T* data() noexcept {
            return m_data;
        }

        const T* data() const noexcept {
            return m_data;
        }

        T* begin() noexcept {
            return m_data;
        }

        T* end() noexcept {
            return m_data + m_size;
        }

    private:
        T* m_data;
        std::size_t m_size;
    };
}

namespace {
    using namespace std::string_view_literals;

    template<typename T>
    void print(const std::basic_string_view<T> what) noexcept {
        ::WriteFile(::GetStdHandle(STD_OUTPUT_HANDLE), what.data(), what.size() * sizeof(T), nullptr, nullptr);
    }

    template<typename T>
    void print(const T* what){
        return print(std::basic_string_view<T>(what));
    }

    std::uint32_t main_impl(){
        const std::size_t nLayouts = ::GetKeyboardLayoutList(0, nullptr);
        ndl::Static_Array<::HKL> currentLayouts{nLayouts};
        const int nListedLayouts = ::GetKeyboardLayoutList(currentLayouts.size(), currentLayouts.data());

        constexpr std::wstring_view ENGLISH_LAYOUT_NAME = L"00000409"sv;

        ndl::Static_Array<wchar_t> buffer(KL_NAMELENGTH);
        for (auto hKeyboardLayout : currentLayouts) {
            ::ActivateKeyboardLayout(hKeyboardLayout, KLF_REORDER);
            if (::GetKeyboardLayoutNameW(buffer.data())) {
                if (ENGLISH_LAYOUT_NAME == buffer.data()) {
                    const bool unloaded = ::UnloadKeyboardLayout(hKeyboardLayout);
                    print(L"English layout unloaded.\n");
                    return 0;
                }
            }
        }
        print(L"English layout wasn't unloaded (none present?).\n");
        return 0;
    }
}

void entry_point(){
    const auto retCode = main_impl();
    ::ExitProcess(retCode);
}
