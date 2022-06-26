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

#ifndef _WIN32
#define HAS_SYSLOG_ 1
#endif

#ifdef HAS_SYSLOG_

#include <aixlog/sinks/sink_base.hpp>

#include <syslog.h>


namespace aixlog
{
namespace sinks
{


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

} // namespace sinks
} // namespace aixlog

#endif
