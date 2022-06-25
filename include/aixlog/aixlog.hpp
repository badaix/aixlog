/***
      __   __  _  _  __     __    ___
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/
    version 1.5.0
    https://github.com/badaix/aixlog

    This file is part of aixlog
    Copyright (C) 2017-2021 Johannes Pohl

    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/

/// inspired by "eater":
/// https://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix

/// TODO: support libfmt? https://fmt.dev/latest/api.html#argument-lists

#pragma once

#ifndef _WIN32
#define HAS_SYSLOG_ 1
#endif

#ifdef __APPLE__
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1012
#define HAS_APPLE_UNIFIED_LOG_ 1
#endif
#endif
#endif

#include <aixlog/filter.hpp>
#include <aixlog/metadata.hpp>
#include <aixlog/sinks/sink_base.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#define USE_FMT 1

#ifdef USE_FMT
#include <fmt/os.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif


#ifdef HAS_APPLE_UNIFIED_LOG_
#include <os/log.h>
#endif

#ifdef HAS_SYSLOG_
#include <syslog.h>
#endif

#ifdef __ANDROID__
// fix for bug "Android NDK __func__ definition is inconsistent with glibc and C++99"
// https://bugs.chromium.org/p/chromium/issues/detail?id=631489
#ifdef __GNUC__
#define AIXLOG_INTERNAL__FUNC __FUNCTION__
#else
#define AIXLOG_INTERNAL__FUNC __func__
#endif
#else
#define AIXLOG_INTERNAL__FUNC __func__
#endif

#define LOG(SEVERITY, TAG, FORMAT, ...)                                                                                                                        \
    aixlog::Log::log(aixlog::Metadata(static_cast<aixlog::Severity>(SEVERITY), TAG, aixlog::Function(AIXLOG_INTERNAL__FUNC, __FILE__, __LINE__),               \
                                      aixlog::Timestamp(std::chrono::system_clock::now())),                                                                    \
                     FMT_STRING(FORMAT), ##__VA_ARGS__)

#define CLOG(SEVERITY, TAG, CONDITION, FORMAT, ...)                                                                                                            \
    aixlog::Log::clog(aixlog::Metadata(static_cast<aixlog::Severity>(SEVERITY), TAG, aixlog::Function(AIXLOG_INTERNAL__FUNC, __FILE__, __LINE__),              \
                                       aixlog::Timestamp(std::chrono::system_clock::now())),                                                                   \
                      CONDITION, FMT_STRING(FORMAT), ##__VA_ARGS__)



namespace aixlog
{



/**
 * @brief
 * For Conditional logging of a log line
 */
struct Conditional
{
    using EvalFunc = std::function<bool()>;

    Conditional() : func_([](void) { return true; })
    {
    }

    Conditional(const EvalFunc& func) : func_(func)
    {
    }

    Conditional(bool value) : func_([value](void) { return value; })
    {
    }

    virtual ~Conditional() = default;

    virtual bool is_true() const
    {
        return func_();
    }

    std::string to_string() const
    {
        if (is_true())
            return "true";
        else
            return "false";
    }

protected:
    EvalFunc func_;
};


static std::ostream& operator<<(std::ostream& os, const Conditional& conditional)
{
    os << conditional.to_string();
    return os;
}



using log_sink_ptr = std::shared_ptr<sinks::Sink>;

/**
 * @brief
 * Main Logger class with "Log::init"
 *
 * Don't use it directly, but call once "Log::init" with your log sink instances.
 * The Log class will simply redirect clog to itself (as a streambuf) and
 * forward whatever went to clog to the log sink instances
 */
class Log
{
public:
    template <typename T, typename... Ts>
    static std::shared_ptr<T> add_logsink(Ts&&... params)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        static_assert(std::is_base_of<sinks::Sink, typename std::decay<T>::type>::value, "type T must be a Sink");
        std::shared_ptr<T> sink = std::make_shared<T>(std::forward<Ts>(params)...);
        state().sinks.push_back(sink);
        return sink;
    }

    static void add_logsink(const log_sink_ptr& sink)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.push_back(sink);
    }

    static void add_logsinks(const std::vector<log_sink_ptr> log_sinks)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.clear();

        for (const auto& sink : log_sinks)
            Log::add_logsink(sink);
    }

    template <typename T, typename... Ts>
    static std::shared_ptr<T> set_logsink(Ts&&... params)
    {
        static_assert(std::is_base_of<sinks::Sink, typename std::decay<T>::type>::value, "type T must be a Sink");
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.clear();
        return Log::add_logsink<T>(std::forward<Ts>(params)...);
    }

    static void set_logsink(const log_sink_ptr& sink)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.clear();
        Log::add_logsink(sink);
    }

    static void set_logsinks(const std::vector<log_sink_ptr> log_sinks)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.clear();
        add_logsinks(log_sinks);
    }

    static void remove_logsink(const log_sink_ptr& sink)
    {
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        state().sinks.erase(std::remove(state().sinks.begin(), state().sinks.end(), sink), state().sinks.end());
    }

    template <typename S, typename... Args>
    static void clog(const Metadata& meta, const aixlog::Conditional& condition, const S& format, Args&&... args)
    {
        if (condition.is_true())
            log(meta, format, std::forward<Args>(args)...);
    }

    template <typename S, typename... Args>
    static void log(const Metadata& meta, const S& format, Args&&... args)
    {
        std::string s;
        std::lock_guard<std::recursive_mutex> lock(state().mutex);
        for (const auto& sink : state().sinks)
        {
            if (sink->filter.match(meta))
            {
                // only construct the string if really needed
                if (s.empty())
                {
                    s = fmt::vformat(format, fmt::make_args_checked<Args...>(format, args...));
                    // Empty log line => no need to log
                    if (s.empty())
                        break;
                }
                sink->log(meta, s);
            }
        }
    }

protected:
    Log() = default;
    virtual ~Log() = default;

private:
    struct State
    {
        std::vector<log_sink_ptr> sinks;
        std::recursive_mutex mutex;
    };

    static State& state()
    {
        static State state;
        return state;
    }
};



#ifdef _WIN32
/**
 * @brief
 * Windows: Logging to OutputDebugString
 *
 * Not tested due to unavailability of Windows
 */
struct SinkOutputDebugString : public Sink
{
    SinkOutputDebugString(const Filter& filter) : Sink(filter)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
#ifdef UNICODE
        std::wstring wide = std::wstring(message.begin(), message.end());
        OutputDebugString(wide.c_str());
#else
        OutputDebugString(message.c_str());
#endif
    }
};
#endif

#ifdef HAS_APPLE_UNIFIED_LOG_
/**
 * @brief
 * macOS: Logging to Apples system logger
 */
struct SinkUnifiedLogging : public Sink
{
    SinkUnifiedLogging(const Filter& filter) : Sink(filter)
    {
    }

    os_log_type_t get_os_log_type(Severity severity) const
    {
        // https://developer.apple.com/documentation/os/os_log_type_t?language=objc
        switch (severity)
        {
            case Severity::trace:
            case Severity::debug:
                return OS_LOG_TYPE_DEBUG;
            case Severity::info:
            case Severity::notice:
                return OS_LOG_TYPE_INFO;
            case Severity::warning:
                return OS_LOG_TYPE_DEFAULT;
            case Severity::error:
                return OS_LOG_TYPE_ERROR;
            case Severity::fatal:
                return OS_LOG_TYPE_FAULT;
            default:
                return OS_LOG_TYPE_DEFAULT;
        }
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        os_log_with_type(OS_LOG_DEFAULT, get_os_log_type(metadata.severity), "%{public}s", message.c_str());
    }
};
#endif

#ifdef HAS_SYSLOG_
/**
 * @brief
 * UNIX: Logging to syslog
 */
struct SinkSyslog : public sinks::Sink
{
    SinkSyslog(const char* ident, const Filter& filter) : Sink(filter)
    {
        openlog(ident, LOG_PID, LOG_USER);
    }

    ~SinkSyslog() override
    {
        closelog();
    }

    int get_syslog_priority(Severity severity) const
    {
        // http://unix.superglobalmegacorp.com/Net2/newsrc/sys/syslog.h.html
        switch (severity)
        {
            case Severity::trace:
            case Severity::debug:
                return LOG_DEBUG;
            case Severity::info:
                return LOG_INFO;
            case Severity::notice:
                return LOG_NOTICE;
            case Severity::warning:
                return LOG_WARNING;
            case Severity::error:
                return LOG_ERR;
            case Severity::fatal:
                return LOG_CRIT;
            default:
                return LOG_INFO;
        }
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        syslog(get_syslog_priority(metadata.severity), "%s", message.c_str());
    }
};
#endif

#ifdef __ANDROID__
/**
 * @brief
 * Android: Logging to android log
 *
 * Use logcat to read the logs
 */
struct SinkAndroid : public Sink
{
    SinkAndroid(const std::string& ident, const Filter& filter) : Sink(filter), ident_(ident)
    {
    }

    android_LogPriority get_android_prio(Severity severity) const
    {
        // https://developer.android.com/ndk/reference/log_8h.html
        switch (severity)
        {
            case Severity::trace:
                return ANDROID_LOG_VERBOSE;
            case Severity::debug:
                return ANDROID_LOG_DEBUG;
            case Severity::info:
            case Severity::notice:
                return ANDROID_LOG_INFO;
            case Severity::warning:
                return ANDROID_LOG_WARN;
            case Severity::error:
                return ANDROID_LOG_ERROR;
            case Severity::fatal:
                return ANDROID_LOG_FATAL;
            default:
                return ANDROID_LOG_UNKNOWN;
        }
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        std::string tag = metadata.tag ? metadata.tag.text : (metadata.function ? metadata.function.name : "");
        std::string log_tag;
        if (!ident_.empty() && !tag.empty())
            log_tag = ident_ + "." + tag;
        else if (!ident_.empty())
            log_tag = ident_;
        else if (!tag.empty())
            log_tag = tag;
        else
            log_tag = "log";

        __android_log_write(get_android_prio(metadata.severity), log_tag.c_str(), message.c_str());
    }

protected:
    std::string ident_;
};
#endif

#ifdef _WIN32
/**
 * @brief
 * Windows: Logging to event logger
 *
 * Not tested due to unavailability of Windows
 */
struct SinkEventLog : public Sink
{
    SinkEventLog(const std::string& ident, const Filter& filter) : Sink(filter)
    {
#ifdef UNICODE
        std::wstring wide = std::wstring(ident.begin(), ident.end()); // stijnvdb: RegisterEventSource expands to RegisterEventSourceW which takes wchar_t
        event_log = RegisterEventSource(NULL, wide.c_str());
#else
        event_log = RegisterEventSource(NULL, ident.c_str());
#endif
    }

    WORD get_type(Severity severity) const
    {
        // https://msdn.microsoft.com/de-de/library/windows/desktop/aa363679(v=vs.85).aspx
        switch (severity)
        {
            case Severity::trace:
            case Severity::debug:
                return EVENTLOG_INFORMATION_TYPE;
            case Severity::info:
            case Severity::notice:
                return EVENTLOG_SUCCESS;
            case Severity::warning:
                return EVENTLOG_WARNING_TYPE;
            case Severity::error:
            case Severity::fatal:
                return EVENTLOG_ERROR_TYPE;
            default:
                return EVENTLOG_INFORMATION_TYPE;
        }
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
#ifdef UNICODE
        std::wstring wide = std::wstring(message.begin(), message.end());
        // We need this temp variable because we cannot take address of rValue
        const auto* c_str = wide.c_str();
        ReportEvent(event_log, get_type(metadata.severity), 0, 0, NULL, 1, 0, &c_str, NULL);
#else
        const auto* c_str = message.c_str();
        ReportEvent(event_log, get_type(metadata.severity), 0, 0, NULL, 1, 0, &c_str, NULL);
#endif
    }

protected:
    HANDLE event_log;
};
#endif

/**
 * @brief
 * Log to the system's native sys logger
 *
 * - Android: Android log
 * - macOS:   unified log
 * - Windows: event log
 * - Unix:    syslog
 */
struct SinkNative : public sinks::Sink
{
    SinkNative(const std::string& ident, const Filter& filter) : Sink(filter), log_sink_(nullptr), ident_(ident)
    {
#ifdef __ANDROID__
        log_sink_ = std::make_shared<SinkAndroid>(ident_, filter);
#elif HAS_APPLE_UNIFIED_LOG_
        log_sink_ = std::make_shared<SinkUnifiedLogging>(filter);
#elif _WIN32
        log_sink_ = std::make_shared<SinkEventLog>(ident, filter);
#elif HAS_SYSLOG_
        log_sink_ = std::make_shared<SinkSyslog>(ident_.c_str(), filter);
#else
        /// will not throw or something. Use "get_logger()" to check for success
        log_sink_ = nullptr;
#endif
    }

    virtual log_sink_ptr get_logger()
    {
        return log_sink_;
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        if (log_sink_ != nullptr)
            log_sink_->log(metadata, message);
    }

protected:
    log_sink_ptr log_sink_;
    std::string ident_;
};

/**
 * @brief
 * Forward log messages to a callback function
 *
 * Pass the callback function to the c'tor.
 * This can be any function that matches the signature of "callback_fun"
 * Might also be a lambda function
 */
struct SinkCallback : public sinks::Sink
{
    using callback_fun = std::function<void(const Metadata& metadata, const std::string& message)>;

    SinkCallback(const Filter& filter, callback_fun callback) : Sink(filter), callback_(callback)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        if (callback_)
            callback_(metadata, message);
    }

private:
    callback_fun callback_;
};


} // namespace aixlog
