/***
      __   __  _  _  __     __    ___
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/

    This file is part of aixlog
    Copyright (C) 2017-2022 Johannes Pohl

    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/


#include "aixlog.hpp"


using namespace std;

static constexpr auto LOG_TAG = "LOG TAG";

/// Log Conditional to log only every x-th message
struct EveryXConditional : public AixLog::Conditional
{
    /// c'tor
    /// @param every_x log only every_x-th line
    EveryXConditional(size_t every_x) : every_x_(every_x), x_th_(0)
    {
    }

    /// check if this is the x-th log message
    /// @return true if this is the x-th log message
    bool is_true() const override
    {
        if (++x_th_ == every_x_)
        {
            x_th_ = 0;
            return true;
        }
        return false;
    }

private:
    size_t every_x_;
    mutable size_t x_th_;
};


int main(int /*argc*/, char** /*argv*/)
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    // AixLog::Log::instance().log("The answer is {}.", 42);
    LOG(TRACE, LOG_TAG, "The answer is {}.", 42);
    LOG(TRACE, LOG_TAG, "Hello, {}!", "world");
    LOG(TRACE, LOG_TAG, "The answer is 42.");

    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    LOG(TRACE, LOG_TAG, "Logger with one cout log sink");
    LOG(DEBUG, LOG_TAG, "Logger with one cout log sink");
    LOG(INFO, LOG_TAG, "Logger with one cout log sink");

    AixLog::Filter filter;
    // log all lines with "trace" severity
    filter.add_filter("*:TRACE");
    // log all lines with tag "LOG_TAG"	with debug or higher severity
    filter.add_filter("LOG_TAG:DEBUG");
    auto sink_cout = make_shared<AixLog::SinkCout>(filter);
    AixLog::Filter filter_syslog;
    // log lines with tag "SYSLOG" to syslog
    filter_syslog.add_filter("SYSLOG:TRACE");
    auto sink_syslog = make_shared<AixLog::SinkNative>("aixlog example", filter_syslog);

    AixLog::Log::init({sink_cout, sink_syslog});

    LOG(TRACE, "LOG_TAG", "Logger with one cout log sink (filtered out)");
    LOG(TRACE, "OTHER TAG", "Logger with one cout log sink (not filtered out)");
    LOG(DEBUG, "SYSLOG", "This will go also to syslog");

    AixLog::Log::init({/// Log everything into file "all.log"
                       make_shared<AixLog::SinkFile>(AixLog::Severity::trace, "all.log"),
                       /// Log everything to SinkCout
                       make_shared<AixLog::SinkCout>(AixLog::Severity::trace, "cout: %Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func) #message"),
                       /// Log error and higher severity messages to cerr
                       make_shared<AixLog::SinkCerr>(AixLog::Severity::error, "cerr: %Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)"),
                       /// Callback log sink with cout logging in a lambda function
                       /// Could also do file logging
                       make_shared<AixLog::SinkCallback>(AixLog::Severity::trace, [](const AixLog::Metadata& metadata, const std::string& message) {
                           cout << "Callback:\n\tmsg:   " << message << "\n\ttag:   " << metadata.tag.text
                                << "\n\tsever: " << AixLog::to_string(metadata.severity) << " (" << static_cast<int>(metadata.severity) << ")\n"
                                << "\ttid:   " << metadata.thread_id << "\n";
                           if (metadata.timestamp)
                               cout << "\ttime:  " << metadata.timestamp.to_string() << "\n";
                           if (metadata.function)
                               cout << "\tfunc:  " << metadata.function.name << "\n\tline:  " << metadata.function.line
                                    << "\n\tfile:  " << metadata.function.file << "\n";
                       })});

#ifdef WIN32
    AixLog::Log::instance().add_logsink<AixLog::SinkOutputDebugString>(AixLog::Severity::trace);
#endif
    LOG(INFO, "guten tag", "LOG(INFO, \"guten tag\")");

    LOG(WARNING, LOG_TAG, "LOG(WARNING)");
    LOG(NOTICE, LOG_TAG, "LOG(NOTICE)");
    LOG(INFO, LOG_TAG, "LOG(INFO)");
    LOG(DEBUG, LOG_TAG, "LOG(DEBUG)");
    LOG(TRACE, LOG_TAG, "LOG(TRACE)");

    AixLog::Severity severity(AixLog::Severity::debug);
    LOG(severity, LOG_TAG, "LOG(severity, LOG_TAG, severity");

    EveryXConditional every_x(3);
    CLOG(INFO, LOG_TAG, every_x, "1st will not be logged");
    CLOG(INFO, LOG_TAG, every_x, "2nd will not be logged");
    CLOG(INFO, LOG_TAG, every_x, "3rd will be logged");
    CLOG(INFO, LOG_TAG, every_x, "4th will not be logged");
    CLOG(INFO, LOG_TAG, every_x, "5th will not be logged");
    CLOG(INFO, LOG_TAG, every_x, "6th will be logged");

    AixLog::Conditional not_every_3(AixLog::Conditional::EvalFunc([] {
        static size_t n(0);
        return (++n % 3 != 0);
    }));

    CLOG(INFO, LOG_TAG, not_every_3, "1st will be logged");
    CLOG(INFO, LOG_TAG, not_every_3, "2nd will be logged");
    CLOG(INFO, LOG_TAG, not_every_3, "3rd will not be logged");
    CLOG(INFO, LOG_TAG, not_every_3, "4th will be logged");
    CLOG(INFO, LOG_TAG, not_every_3, "5th will be logged");
    CLOG(INFO, LOG_TAG, not_every_3, "6th will not be logged");

    CLOG(INFO, LOG_TAG, false, "will not be logged");
    CLOG(INFO, LOG_TAG, true, "will be logged");
}
