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

/// TODO: support libfmt? https://fmt.dev/latest/api.html#argument-lists

#pragma once

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


} // namespace aixlog
