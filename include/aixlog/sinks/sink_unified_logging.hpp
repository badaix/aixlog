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


#ifdef __APPLE__
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1012
#define HAS_APPLE_UNIFIED_LOG_ 1
#endif
#endif
#endif

#pragma once

#ifdef HAS_APPLE_UNIFIED_LOG_

#include <aixlog/sinks/sink_base.hpp>

#ifdef HAS_APPLE_UNIFIED_LOG_
#include <os/log.h>
#endif

namespace aixlog
{
namespace sinks
{

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


} // namespace sinks
} // namespace aixlog

#endif
