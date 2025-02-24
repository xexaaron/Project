#include "Platform/Platform.h"
#include "Core/Log.h"
#include <iostream>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    #define POSIX 
#endif

#ifdef _WIN32  // Windows
    #include "Platform/win32/PlatformWin32.h"
    #define PLATFORM_NAMESPACE win32
#elif defined(POSIX)
    #include "Platform/posix/PlatformPosix.h"
    #define PLATFORM_NAMESPACE posix
#else
    #error "Unsupported platform"
#endif

namespace aby::sys {
    
    auto is_terminal(const std::ostream& os) -> bool {
        FILE* stream = nullptr;
        if (&os == &std::cout) {
            stream = stdout;
        }
        else if (&os == &std::cerr) {
            stream = stderr;
        }
        else {
            ABY_ERR("std::ostream is not cout or cerr");
        }
        return PLATFORM_NAMESPACE::is_terminal(stream);
    }

    auto set_thread_name(std::thread& thread, const std::string& name) -> bool {
        return PLATFORM_NAMESPACE::set_thread_name(thread, name);
    }

    auto get_exec_path() -> fs::path {
        return PLATFORM_NAMESPACE::get_exec_path();
    }

    auto set_cursor(ECursor cursor) -> bool {
        return PLATFORM_NAMESPACE::set_cursor(cursor);
    }

}