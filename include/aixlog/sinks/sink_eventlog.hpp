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

#ifdef _WIN32

#include <aixlog/sinks/sink_base.hpp>

namespace aixlog
{
namespace sinks
{

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

} // namespace sinks
} // namespace aixlog

#endif
