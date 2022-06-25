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

#pragma once


#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>


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

namespace aixlog
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

} // namespace aixlog


#ifdef _WIN32
// We restore the ERROR Windows macro
#pragma pop_macro("ERROR")
#pragma pop_macro("DEBUG")
#endif
