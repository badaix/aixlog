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

#ifndef AIX_LOG_HPP
#define AIX_LOG_HPP

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

#include <algorithm>
// #include <cctype>
#include <chrono>
// #include <cstdio>
// #include <ctime>
#include <fstream>
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

#ifdef _WIN32
#include <Windows.h>
// ERROR macro is defined in Windows header
// To avoid conflict between these macro and declaration of ERROR / DEBUG in SEVERITY enum
// We save macro and undef it
#pragma push_macro("ERROR")
#pragma push_macro("DEBUG")
#undef ERROR
#undef DEBUG
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
    AixLog::Log::log(AixLog::Metadata(static_cast<AixLog::Severity>(SEVERITY), TAG, AixLog::Function(AIXLOG_INTERNAL__FUNC, __FILE__, __LINE__),               \
                                      AixLog::Timestamp(std::chrono::system_clock::now())),                                                                    \
                     FMT_STRING(FORMAT), ##__VA_ARGS__)

#define CLOG(SEVERITY, TAG, CONDITION, FORMAT, ...)                                                                                                            \
    AixLog::Log::clog(AixLog::Metadata(static_cast<AixLog::Severity>(SEVERITY), TAG, AixLog::Function(AIXLOG_INTERNAL__FUNC, __FILE__, __LINE__),              \
                                       AixLog::Timestamp(std::chrono::system_clock::now())),                                                                   \
                      CONDITION, FMT_STRING(FORMAT), ##__VA_ARGS__)



/**
 * @brief
 * Severity of the log message
 */
enum SEVERITY
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    NOTICE = 3,
    WARNING = 4,
    ERROR = 5,
    FATAL = 6
};

namespace AixLog
{

/**
 * @brief
 * Severity of the log message
 *
 * Mandatory parameter for the LOG macro
 */
enum class Severity : std::int8_t
{
    // Mapping table from AixLog to other loggers. Boost is just for information.
    // https://chromium.googlesource.com/chromium/mini_chromium/+/master/base/logging.cc
    //
    // Aixlog      Boost       Syslog      Android     macOS       EventLog      Syslog Desc
    //
    // trace       trace       DEBUG       VERBOSE     DEBUG       INFORMATION
    // debug       debug       DEBUG       DEBUG       DEBUG       INFORMATION   debug-level message
    // info        info        INFO        INFO        INFO        SUCCESS       informational message
    // notice                  NOTICE      INFO        INFO        SUCCESS       normal, but significant, condition
    // warning     warning     WARNING     WARN        DEFAULT     WARNING       warning conditions
    // error       error       ERROR       ERROR       ERROR       ERROR         error conditions
    // fatal       fatal       CRIT        FATAL       FAULT       ERROR         critical conditions
    //                         ALERT                                             action must be taken immediately
    //                         EMERG                                             system is unusable

    trace = SEVERITY::TRACE,
    debug = SEVERITY::DEBUG,
    info = SEVERITY::INFO,
    notice = SEVERITY::NOTICE,
    warning = SEVERITY::WARNING,
    error = SEVERITY::ERROR,
    fatal = SEVERITY::FATAL
};


static Severity to_severity(std::string severity, Severity def = Severity::info)
{
    std::transform(severity.begin(), severity.end(), severity.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (severity == "trace")
        return Severity::trace;
    else if (severity == "debug")
        return Severity::debug;
    else if (severity == "info")
        return Severity::info;
    else if (severity == "notice")
        return Severity::notice;
    else if (severity == "warning")
        return Severity::warning;
    else if (severity == "error")
        return Severity::error;
    else if (severity == "fatal")
        return Severity::fatal;
    else
        return def;
}

static std::string to_string(Severity logSeverity)
{
    switch (logSeverity)
    {
        case Severity::trace:
            return "Trace";
        case Severity::debug:
            return "Debug";
        case Severity::info:
            return "Info";
        case Severity::notice:
            return "Notice";
        case Severity::warning:
            return "Warn";
        case Severity::error:
            return "Error";
        case Severity::fatal:
            return "Fatal";
        default:
            std::stringstream ss;
            ss << static_cast<int>(logSeverity);
            return ss.str();
    }
}

static std::ostream& operator<<(std::ostream& os, const Severity& log_severity)
{
    os << to_string(log_severity);
    return os;
}



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


/**
 * @brief
 * Timestamp of a log line
 *
 * to_string will convert the time stamp into a string, using the strftime syntax
 */
struct Timestamp
{
    using time_point_sys_clock = std::chrono::time_point<std::chrono::system_clock>;

    Timestamp(std::nullptr_t) : is_null_(true)
    {
    }

    Timestamp() : Timestamp(nullptr)
    {
    }

    Timestamp(const time_point_sys_clock& time_point) : time_point(time_point), is_null_(false)
    {
    }

    Timestamp(time_point_sys_clock&& time_point) : time_point(std::move(time_point)), is_null_(false)
    {
    }

    virtual ~Timestamp() = default;

    explicit operator bool() const
    {
        return !is_null_;
    }

    /// strftime format + proprietary "#ms" for milliseconds
    std::string to_string(const std::string& format = "%Y-%m-%d %H-%M-%S.#ms") const
    {
        std::time_t now_c = std::chrono::system_clock::to_time_t(time_point);
        struct ::tm now_tm = localtime_xp(now_c);
        char buffer[256];
        strftime(buffer, sizeof buffer, format.c_str(), &now_tm);
        std::string result(buffer);
        size_t pos = result.find("#ms");
        if (pos != std::string::npos)
        {
            int ms_part = std::chrono::time_point_cast<std::chrono::milliseconds>(time_point).time_since_epoch().count() % 1000;
            char ms_str[4];
            if (snprintf(ms_str, 4, "%03d", ms_part) >= 0)
                result.replace(pos, 3, ms_str);
        }
        return result;
    }

    time_point_sys_clock time_point;

private:
    bool is_null_;

    inline std::tm localtime_xp(std::time_t timer) const
    {
        std::tm bt;
#if defined(__unix__)
        localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
#endif
        return bt;
    }
};


static std::ostream& operator<<(std::ostream& os, const Timestamp& timestamp)
{
    os << timestamp.to_string();
    return os;
}

/**
 * @brief
 * Tag (string) for log line
 */
struct Tag
{
    Tag(std::nullptr_t) : text(""), is_null_(true)
    {
    }

    Tag() : Tag(nullptr)
    {
    }

    Tag(const char* text) : text(text), is_null_(false)
    {
    }

    Tag(const std::string& text) : text(text), is_null_(false)
    {
    }

    Tag(std::string&& text) : text(std::move(text)), is_null_(false)
    {
    }

    virtual ~Tag() = default;

    explicit operator bool() const
    {
        return !is_null_;
    }

    bool operator<(const Tag& other) const
    {
        return (text < other.text);
    }

    std::string text;

private:
    bool is_null_;
};


static std::ostream& operator<<(std::ostream& os, const Tag& tag)
{
    os << tag.text;
    return os;
}



/**
 * @brief
 * Capture function, file and line number of the log line
 */
struct Function
{
    Function(const std::string& name, const std::string& file, size_t line) : name(name), file(file), line(line), is_null_(false)
    {
    }

    Function(std::string&& name, std::string&& file, size_t line) : name(std::move(name)), file(std::move(file)), line(line), is_null_(false)
    {
    }

    Function(std::nullptr_t) : name(""), file(""), line(0), is_null_(true)
    {
    }

    Function() : Function(nullptr)
    {
    }

    virtual ~Function() = default;

    explicit operator bool() const
    {
        return !is_null_;
    }

    std::string name;
    std::string file;
    size_t line;

private:
    bool is_null_;
};


static std::ostream& operator<<(std::ostream& os, const Function& function)
{
    os << function.name;
    return os;
}


/**
 * @brief
 * Collection of a log line's meta data
 */
struct Metadata
{
    Metadata() : severity(Severity::trace), tag(nullptr), function(nullptr), timestamp(std::chrono::system_clock::now()), thread_id(std::this_thread::get_id())
    {
    }

    Metadata(Severity severity, Tag tag, Function function, Timestamp timestamp)
        : severity(std::move(severity)), tag(std::move(tag)), function(std::move(function)), timestamp(std::move(timestamp)),
          thread_id(std::this_thread::get_id())
    {
    }

    Severity severity;
    Tag tag;
    Function function;
    Timestamp timestamp;
    std::thread::id thread_id;
};


class Filter
{
public:
    Filter()
    {
    }

    Filter(Severity severity)
    {
        add_filter(severity);
    }

    bool match(const Metadata& metadata) const
    {
        if (tag_filter_.empty())
            return true;

        auto iter = tag_filter_.find(metadata.tag);
        if (iter != tag_filter_.end())
            return (metadata.severity >= iter->second);

        iter = tag_filter_.find("*");
        if (iter != tag_filter_.end())
            return (metadata.severity >= iter->second);

        return false;
    }

    void add_filter(const Tag& tag, Severity severity)
    {
        tag_filter_[tag] = severity;
    }

    void add_filter(Severity severity)
    {
        tag_filter_["*"] = severity;
    }

    void add_filter(const std::string& filter)
    {
        auto pos = filter.find(":");
        if (pos != std::string::npos)
            add_filter(filter.substr(0, pos), to_severity(filter.substr(pos + 1)));
        else
            add_filter(to_severity(filter));
    }

private:
    std::map<Tag, Severity> tag_filter_;
};


/**
 * @brief
 * Abstract log sink
 *
 * All log sinks must inherit from this Sink
 */
struct Sink
{
    Sink(const Filter& filter) : filter(filter)
    {
    }

    virtual ~Sink() = default;

    virtual void log(const Metadata& metadata, const std::string& message) = 0;

    Filter filter;
};

using log_sink_ptr = std::shared_ptr<Sink>;

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
        static_assert(std::is_base_of<Sink, typename std::decay<T>::type>::value, "type T must be a Sink");
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
        static_assert(std::is_base_of<Sink, typename std::decay<T>::type>::value, "type T must be a Sink");
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
    static void clog(const Metadata& meta, const AixLog::Conditional& condition, const S& format, Args&&... args)
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


/**
 * @brief
 * Null log sink
 *
 * Discards all log messages
 */
struct SinkNull : public Sink
{
    SinkNull() : Sink(Filter())
    {
    }

    void log(const Metadata& /*metadata*/, const std::string& /*message*/) override
    {
    }
};


/**
 * @brief
 * Abstract log sink with support for formatting log message
 *
 * "format" in the c'tor defines a log pattern.
 * For every log message, these placeholders will be substituded:
 * - strftime syntax is used to format the logging time stamp (%Y, %m, %d, ...)
 * - #ms: milliseconds part of the logging time stamp with leading zeros
 * - #severity: log severity
 * - #tag_func: the log tag. If empty, the function
 * - #tag: the log tag
 * - #function: the function
 * - #message: the log message
 */
struct SinkFormat : public Sink
{
    SinkFormat(const Filter& filter, const std::string& format) : Sink(filter), format_(format)
    {
    }

    virtual void set_format(const std::string& format)
    {
        format_ = format;
    }

    void log(const Metadata& metadata, const std::string& message) override = 0;

protected:
    virtual void do_log(std::ostream& stream, const Metadata& metadata, const std::string& message) const
    {
        std::string result = format_;
        if (metadata.timestamp)
            result = metadata.timestamp.to_string(result);

        size_t pos = result.find("#severity");
        if (pos != std::string::npos)
            result.replace(pos, 9, to_string(metadata.severity));

        pos = result.find("#tag_func");
        if (pos != std::string::npos)
            result.replace(pos, 9, metadata.tag ? metadata.tag.text : (metadata.function ? metadata.function.name : "log"));

        pos = result.find("#tag");
        if (pos != std::string::npos)
            result.replace(pos, 4, metadata.tag ? metadata.tag.text : "");

        pos = result.find("#function");
        if (pos != std::string::npos)
            result.replace(pos, 9, metadata.function ? metadata.function.name : "");

        pos = result.find("#message");
        if (pos != std::string::npos)
        {
            result.replace(pos, 8, message);
            stream << result << std::endl;
        }
        else
        {
            if (result.empty() || (result.back() == ' '))
                stream << result << message << std::endl;
            else
                stream << result << " " << message << std::endl;
        }
    }

    std::string format_;
};

/**
 * @brief
 * Formatted logging to cout
 */
struct SinkCout : public SinkFormat
{
    SinkCout(const Filter& filter, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)") : SinkFormat(filter, format)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        do_log(std::cout, metadata, message);
    }
};

/**
 * @brief
 * Formatted logging to cerr
 */
struct SinkCerr : public SinkFormat
{
    SinkCerr(const Filter& filter, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)") : SinkFormat(filter, format)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        do_log(std::cerr, metadata, message);
    }
};

/**
 * @brief
 * Formatted logging to file
 */
struct SinkFile : public SinkFormat
{
    SinkFile(const Filter& filter, const std::string& filename, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)")
        : SinkFormat(filter, format)
    {
        ofs.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    }

    ~SinkFile() override
    {
        ofs.close();
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        do_log(ofs, metadata, message);
    }

protected:
    mutable std::ofstream ofs;
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
struct SinkSyslog : public Sink
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
struct SinkNative : public Sink
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
struct SinkCallback : public Sink
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


} // namespace AixLog

#ifdef _WIN32
// We restore the ERROR Windows macro
#pragma pop_macro("ERROR")
#pragma pop_macro("DEBUG")
#endif

#endif // AIX_LOG_HPP
