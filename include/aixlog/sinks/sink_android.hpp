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

#ifdef __ANDROID__

#include <aixlog/sinks/sink_base.hpp>

#include <android/log.h>


namespace aixlog
{
namespace sinks
{


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

} // namespace sinks
} // namespace aixlog

#endif
