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

#include <aixlog/sinks/sink_base.hpp>
#ifdef __ANDROID__
#include <aixlog/sinks/sink_android.hpp>
#elif HAS_APPLE_UNIFIED_LOG_
#include <aixlog/sinks/sink_unified_logging.hpp>
#elif _WIN32
#include <aixlog/sinks/sink_eventlog.hpp>
#elif HAS_SYSLOG_
#include <aixlog/sinks/sink_syslog.hpp>
#endif

namespace aixlog
{
namespace sinks
{

using log_sink_ptr = std::shared_ptr<sinks::Sink>;

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


} // namespace sinks
} // namespace aixlog
