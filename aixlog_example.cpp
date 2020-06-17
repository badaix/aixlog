/***
      __   __  _  _  __     __    ___
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/

    This file is part of aixlog
    Copyright (C) 2017-2020 Johannes Pohl

    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/


#include "aixlog.hpp"

using namespace std;


int main(int /*argc*/, char** /*argv*/)
{
    AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace);
    LOG(TRACE, "LOG_TAG") << "Logger with one cout log sink\n";
    LOG(DEBUG, "LOG_TAG") << "Logger with one cout log sink\n";
    LOG(INFO, "LOG_TAG") << "Logger with one cout log sink\n";

    AixLog::Filter filter;
    // log all lines with "trace" severity
    filter.add_filter("*:TRACE");
    // log all lines with tag "LOG_TAG"	with debug or higher severity
    filter.add_filter("LOG_TAG:DEBUG");
    auto sink_cout = make_shared<AixLog::SinkCout>(filter);
    AixLog::Filter filter_syslog;
	// log lines with tag "SYSLOG" to syslog
	filter_syslog.add_filter("SYSLOG:TRACE");
    auto sink_syslog = make_shared<AixLog::SinkSyslog>("aixlog example", filter_syslog);

    AixLog::Log::init({sink_cout, sink_syslog});

    LOG(TRACE, "LOG_TAG") << "Logger with one cout log sink (filtered out)\n";
    LOG(TRACE, "OTHER TAG") << "Logger with one cout log sink (not filtered out)\n";
    LOG(DEBUG, "SYSLOG") << "Ths will go also to syslog\n";

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
                                << "\n\tsever: " << AixLog::to_string(metadata.severity) << " (" << static_cast<int>(metadata.severity) << ")\n";
                           if (metadata.timestamp)
                               cout << "\ttime:  " << metadata.timestamp.to_string() << "\n";
                           if (metadata.function)
                               cout << "\tfunc:  " << metadata.function.name << "\n\tline:  " << metadata.function.line
                                    << "\n\tfile:  " << metadata.function.file << "\n";
                       })});

    /// Log with info severity
    LOG(INFO) << "LOG(INFO)\n";
    /// ... with a tag
    LOG(INFO, "guten tag") << "LOG(INFO, \"guten tag\")\n";
    /// ... with an explicit tag (same result as above)
    LOG(INFO) << TAG("guten tag") << "LOG(INFO) << TAG(\"guten tag\")\n";

    /// Different log severities
    LOG(FATAL) << "LOG(FATAL)\nLOG(FATAL) Second line\n";
    LOG(FATAL) << TAG("hello") << "LOG(FATAL) << TAG(\"hello\") no line break";
    LOG(FATAL) << "LOG(FATAL) 2 no line break";
    LOG(ERROR) << "LOG(ERROR): change in log-level will add a line break";
    LOG(WARNING) << "LOG(WARNING)";
    LOG(NOTICE) << "LOG(NOTICE)";
    LOG(INFO) << "LOG(INFO)\n";
    LOG(INFO) << TAG("my tag") << "LOG(INFO) << TAG(\"my tag\")\n";
    LOG(DEBUG) << "LOG(DEBUG)\n";
    LOG(TRACE) << "LOG(TRACE)\n";

    /// Conditional logging
    LOG(DEBUG) << COND(1 == 1) << "LOG(DEBUG) will be logged\n";
    LOG(DEBUG) << COND(1 == 2) << "LOG(DEBUG) will not be logged\n";

    /// Colors :-)
    LOG(FATAL) << "LOG(FATAL) " << AixLog::Color::red << "red" << AixLog::Color::none << ", default color\n";
    LOG(FATAL) << "LOG(FATAL) " << COLOR(red) << "red" << COLOR(none) << ", default color (using macros)\n";
    LOG(FATAL) << "LOG(FATAL) " << AixLog::TextColor(AixLog::Color::yellow, AixLog::Color::blue) << "yellow on blue background" << AixLog::Color::none
               << ", default color\n";
    LOG(FATAL) << "LOG(FATAL) " << COLOR(yellow, blue) << "yellow on blue background" << COLOR(none) << ", default color (using macros)\n";

    AixLog::Severity severity(AixLog::Severity::debug);
    LOG(severity) << "LOG(severity) << severity\n";

    LOG(INFO, "TAG_A") << "LOG(INFO, TAG_A)\n";
    LOG(DEBUG, "TAG_A") << "LOG(DEBUG, TAG_B)\n";
    LOG(TRACE, "TAG_A") << "LOG(TRACE)\n";

    LOG(INFO, "TAG_A") << "LOG(INFO, TAG_A)\n";
    LOG(DEBUG, "TAG_A") << "LOG(DEBUG, TAG_B)\n";
    LOG(TRACE, "TAG_A") << "LOG(TRACE)\n";
}
