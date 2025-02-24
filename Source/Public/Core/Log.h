#pragma once

#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <tuple>
#include <source_location>
#include <filesystem>
#include <functional>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "Core/Common.h"

#ifndef ABY_LOG
#define ABY_LOG(...) aby::Logger::log(__VA_ARGS__)
#endif
#ifndef ABY_ERR
#define ABY_ERR(...) aby::Logger::error(__VA_ARGS__)
#endif
#ifndef ABY_WARN
#define ABY_WARN(...) aby::Logger::warn(__VA_ARGS__)
#endif
#ifndef NDEBUG
    #ifndef ABY_DBG
        #define ABY_DBG(...) aby::Logger::debug(__VA_ARGS__)
    #endif
  #ifndef ABY_ASSERT
#define ABY_ASSERT(condition, ...)                                                   \
    do {                                                                             \
        if (!(condition)) {                                                          \
            aby::Logger::Assert("File:{}:{}\n{}",                                    \
                std::string_view(__FILE__).substr(                                   \
                    std::string_view(__FILE__).find_last_of("/\\") + 1),             \
                __LINE__,                                                            \
                ABY_FUNC_SIG                                                         \
            );                                                                       \
            aby::Logger::Assert("!({})\n{}", #condition, std::format(__VA_ARGS__));  \
            ABY_DBG_BREAK();                                                         \
        }                                                                            \
    } while (0)
#endif
#else
    #ifndef ABY_DBG
        #define ABY_DBG(...)
    #endif

    #ifndef ABY_ASSERT
        #define ABY_ASSERT(...)
    #endif
#endif



namespace aby {

    enum class ELogColor {
        Grey      = 232,
        Cyan      = 36,
        Yellow    = 33,
        Red       = 91,
    };

    enum class ELogLevel {
        LOG    = 232,
        DEBUG  = 36,
        WARN   = 33,
        ERR    = 91,
        ASSERT = ERR,
    };

    struct LogMsg {
        ELogLevel   level;
        std::string text;
        ELogColor color() const { return static_cast<ELogColor>(level); }
    };

    class Logger {
    private:
        template <typename... Args>
        inline static void print(const std::string& context, ELogColor  color, std::format_string<Args...> fmt, Args&&... args) {
            std::string prefix = std::format("[{}]", context);
            std::string msg    = std::format(fmt, std::forward<Args>(args)...);
            std::string out    = prefix.append(" ").append(msg);
            m_MsgBuff[m_MsgCount] = LogMsg{ .level = static_cast<ELogLevel>(color), .text = out };
            m_MsgCount++;
            if (m_MsgCount == m_MsgBuff.size()) {
                flush();
            }
          
        }
    public:
        using Callback = std::function<void(LogMsg)>;

        static void set_streams(std::ostream& log_stream = std::clog, std::ostream& err_stream = std::cerr) {
            m_LogStream = &log_stream;
            m_ErrStream = &err_stream;
        }

        static std::size_t add_callback(Callback&& callback) {
            std::lock_guard lock(m_StreamMutex);
            std::size_t idx = m_Callbacks.size();
            m_Callbacks.push_back(callback);
            return idx;
        }
        static void remove_callback(std::size_t idx) {
            m_Callbacks.erase(m_Callbacks.begin() + idx);
        }
        static void flush() {
            bool unlock = m_StreamMutex.try_lock();
            for (std::size_t i = 0; i < m_MsgCount; i++) {
                auto& msg = m_MsgBuff[i];
                switch (msg.level) {
                    case ELogLevel::LOG:
                    case ELogLevel::DEBUG:
                    {
                        *m_LogStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                        break;
                    }
                    case ELogLevel::WARN:
                    case ELogLevel::ERR:
                    {
                        *m_ErrStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                        break;
                    }
                }
                for (auto& cb : m_Callbacks) {
                    cb(msg);
                }
            }
            m_MsgCount = 0;
            if (unlock) {
                m_StreamMutex.unlock();
            }
        }

        template <typename... Args>
        static void log(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            print("Info", ELogColor::Grey, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        static void warn(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            print("Warn", ELogColor::Yellow, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void error(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            print("Error", ELogColor::Red, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void Assert(std::format_string<Args...> fmt, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            print("Assert", ELogColor::Red, fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        static void debug(std::format_string<Args...> fmt, Args&&... args) {
        #ifndef NDEBUG
            std::lock_guard<std::mutex> lock(m_StreamMutex);
            print("Debug", ELogColor::Cyan, fmt, std::forward<Args>(args)...);
        #endif
        }
    private:
        static inline std::ostream* m_LogStream = &std::clog;
        static inline std::ostream* m_ErrStream = &std::cerr;
        static inline std::mutex    m_StreamMutex;
        static inline std::vector<Callback> m_Callbacks = {};
        static inline std::array<LogMsg, 128> m_MsgBuff = {};
        static inline std::size_t m_MsgCount = 0;
    };

} 

namespace std {
    template <>
    struct formatter<source_location> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }
        
        template <typename FmtContext>
        FmtContext::iterator format(const source_location& loc, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}:{} {}", loc.file_name(), loc.line(), loc.function_name());
        }
    };

    template <>
    struct formatter<filesystem::path> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        FmtContext::iterator format(const filesystem::path& path, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}", path.string());
        }
    };

    template <>
    struct formatter<aby::UUID> {
        template<class ParseContext>
        constexpr ParseContext::iterator parse(ParseContext& ctx) {
            return ctx.begin();
        }

        template <typename FmtContext>
        FmtContext::iterator format(const aby::UUID& uuid, FmtContext& ctx) const {
            return format_to(ctx.out(), "{}", uuid.operator unsigned long long());
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::ivec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, const glm::uvec4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

}


