#include "Platform/win32/PlatformWin32.h"
#include "Core/Log.h"

#ifdef _WIN32

#include <processthreadsapi.h>

#ifdef _MSVC_VER
	#include <io.h> // _isatty, _fileno
#endif

namespace aby::sys::win32 {
    
    auto to_string(const std::wstring& wstring) -> std::string {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) {
            return "";
        }
        std::string str(size_needed - 1, '\0'); // Remove null terminator
        WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
        return str;
    }

    auto to_wstring(const std::string& string) -> std::wstring {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);
        if (size_needed <= 0) {
            return L"";
        }
        std::wstring wstring(size_needed - 1, L'\0'); // Remove null terminator
        MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, &wstring[0], size_needed);
        return wstring;
    }

    auto is_terminal(std::FILE* stream) -> bool  {
    #ifdef _MSVC_VER
        return _isattty(_fileno(stream));
    #elif defined(_WIN32)
        DWORD mode;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return false;
        return GetConsoleMode(hOut, &mode);
    #endif
	}

    auto set_thread_name(std::thread& thread, const std::string& name) -> bool {
        HANDLE handle = reinterpret_cast<HANDLE>(thread.native_handle());
        if (name.size() > 15) { 
            wchar_t* wdesc = nullptr;
            if (SUCCEEDED(GetThreadDescription(handle, &wdesc))) {
                std::string desc = wdesc ? to_string(wdesc) : "<unknown>";
                LocalFree(wdesc);
                if (name.size() > 15) {
                    ABY_ERR("Thread name exceeds 15 characters (legacy limit). Thread: {}, Name: {}", desc, name);
                }
            }
        }
        std::wstring wname = to_wstring(name);
        if (FAILED(SetThreadDescription(handle, wname.c_str()))) {
            return false;
        }
        return true;
    }

    auto get_exec_path() -> fs::path {
        char path_buff[MAX_PATH];
        ABY_ASSERT(GetModuleFileName(NULL, path_buff, MAX_PATH) != 0, "");
        return fs::path(path_buff);
    }

    auto set_cursor(ECursor cursor) -> bool {
        decltype(IDC_ARROW) wcursor = IDC_ARROW;
        switch (cursor) {
            case ECursor::ARROW:
                break;
            case ECursor::IBEAM:
                wcursor = ((LPSTR)((ULONG_PTR)((WORD)(32513)))); break;
            case ECursor::CROSSHAIR:
                wcursor = ((LPSTR)((ULONG_PTR)((WORD)(32515)))); break;
            case ECursor::HAND:
                wcursor = ((LPSTR)((ULONG_PTR)((WORD)(32649)))); break;
            case ECursor::HRESIZE:
                wcursor = ((LPSTR)((ULONG_PTR)((WORD)(32644)))); break;
            case ECursor::VRESIZE:
                wcursor = ((LPSTR)((ULONG_PTR)((WORD)(32645)))); break;
            default:
                break;
        }

        HCURSOR hcursor = LoadCursor(nullptr, MAKEINTRESOURCE(wcursor));
        if (hcursor) {
            ::SetCursor(hcursor);
            return true;
        }

        return false;
    }


    auto get_pid() -> int {
        return GetCurrentProcessId();
    }

}

#endif