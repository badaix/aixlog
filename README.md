# AixLog

Header-only C++ logging library

## Features
* Single header file implementation
  * Simply include and use it!
  * No dependcies, just vanilla C++11
* Permissive MIT license
* Use ostream operator `<<`
  * Unobtrusive, typesafe and expressive
  * Easy to switch from existing "cout logging"
* Fancy name
* Native support for various platforms (through Sinks)
  * Linux, Unix: Syslog
  * macOS: Unified logging (os_log)
  * Android: Android Log
  * Windows: Event log, OutputDebugString
* Several Sinks:
  * cout
  * cerr
  * Sink with custom callback function
    * implement your own log sink in a lambda with a single line of code
  * Easy to add more...
* Manipulators for
  * Different log levels: `TRACE, DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL`  
    `LOG(ERROR) << "some error happened!"`  
    `LOG(DEBUG) << "Just a debug message"`
  * Conditional logging: simply put `COND(bool)` in front   
    `LOG(INFO) << COND(false) << "will not be logged\n"`  
    `LOG(INFO) << COND(true) << "will be logged\n"`
  * Support for tags:  
    `LOG(INFO, "my tag") << "some message"`  
    ...is the same as...  
    `LOG(INFO) << TAG("my tag") << "some message"`
  * Capture function and line number and timestamp
  * Two different log types "normal" and "special": `LOG(INFO) << SPECIAL << "some special message"`
    * special might be used for syslog, while normal is used for console output
    * => Only special tagged messages will go to syslog
  * Support for colors:
    * Foreground: `LOG(INFO) << COLOR(red) << "red foreground"`
    * Foreground and background: `LOG(INFO) << COLOR(yellow, blue) << "yellow on blue background"`

## Basic usage
To use AixLog, you must once pass a log sink to `AixLog:Log::init`, e.g. a `SinkCout`:
```c++
AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace, AixLog::Type::normal);
LOG(INFO) << "Hello, World!\n";
```
This will print
```
2017-09-28 11-01-16.049 [Info] (main) Hello, World!
```
There are two overloads for `AixLog:Log::init`: 
1. one creates and returns an instance of a Sink (as in the example above)
```c++
auto sink = AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::trace, AixLog::Type::normal);
```
2. one takes a vector of Sinks
```c++
auto sink_cout = make_shared<AixLog::SinkCout>(AixLog::Severity::trace, AixLog::Type::normal);
auto sink_file = make_shared<AixLog::SinkFile>(AixLog::Severity::trace, AixLog::Type::all, "logfile.log");
AixLog::Log::init({sink_cout, sink_file});
```


## Advanced usage example
```c++
#include "aixlog.hpp"

using namespace std;

int main(int argc, char** argv)
{
	AixLog::Log::init(
		{
			/// Log normal (i.e. non-special) logs to SinkCout
			make_shared<AixLog::SinkCout>(AixLog::Severity::trace, AixLog::Type::normal, "cout: %Y-%m-%d %H-%M-%S.#ms [#severity] (#tag) #message"),
			/// Log error and higher severity messages to cerr
			make_shared<AixLog::SinkCerr>(AixLog::Severity::error, AixLog::Type::all, "cerr: %Y-%m-%d %H-%M-%S.#ms [#severity] (#tag)"),
			/// Log special logs to native log (Syslog on Linux, Android Log on Android, EventLog on Windows, Unified logging on Apple)
			make_shared<AixLog::SinkNative>("aixlog", AixLog::Severity::trace, AixLog::Type::special),
			/// Callback log sink with cout logging in a lambda function
			/// Could also do file logging
			make_shared<AixLog::SinkCallback>(AixLog::Severity::trace, AixLog::Type::all, 
				[](const AixLog::Metadata& metadata, const std::string& message)
				{
					cout << "Callback:\n\tmsg:   " << message << "\n\ttag:   " << metadata.tag.text << "\n\tsever: " << AixLog::Log::to_string(metadata.severity) << " (" << (int)metadata.severity << ")\n\ttype:  " << (metadata.type == AixLog::Type::normal?"normal":"special") << "\n";
					if (metadata.timestamp)
						cout << "\ttime:  " << metadata.timestamp.to_string() << "\n";
					if (metadata.function)
						cout << "\tfunc:  " << metadata.function.name << "\n\tline:  " << metadata.function.line << "\n\tfile:  " << metadata.function.file << "\n";
				}
			)
		}
	);

	/// Log with info severity
	LOG(INFO) << "LOG(INFO)\n";
	/// ... with a tag
	LOG(INFO, "guten tag") << "LOG(INFO, \"guten tag\")\n";
	/// ... with an explicit tag (same result as above)
	LOG(INFO) << TAG("guten tag") << "LOG(INFO) << TAG(\"guten tag\")\n";
	/// Log "special" with info severity
	SLOG(INFO) << "SLOG(INFO)\n";
	/// Log with explicit "special" type
	LOG(INFO) << AixLog::Type::special << "LOG(INFO) << AixLog::Type::special\n";
	/// Log with explicit "special" type (now with a macro)
	LOG(INFO) << SPECIAL << "LOG(INFO) << SPECIAL\n";
	/// ... with explicit "special" type and explicit tag
	LOG(INFO) << SPECIAL << TAG("guten tag") << "LOG(INFO) << SPECIAL << TAG(\"guten tag\")\n";

	/// Different log severities
	LOG(FATAL) << "LOG(FATAL)\nLOG(FATAL) Second line\n";
	LOG(FATAL) << TAG("hello") << "LOG(FATAL) << TAG(\"hello\") no line break";
	LOG(FATAL) << "LOG(FATAL) 2 no line break";
	LOG(ERROR) << "LOG(ERROR): change in log-level will add a line break";
	LOG(WARNING) << "LOG(WARNING)";
	LOG(NOTICE) << "LOG(NOTICE)";
	LOG(INFO) << "LOG(INFO)\n";
	LOG(INFO) << TAG("my tag") << "LOG(INFO) << TAG(\"my tag\")n";
	LOG(DEBUG) << "LOG(DEBUG)\n";
	LOG(TRACE) << "LOG(TRACE)\n";

	/// Conditional logging
	LOG(DEBUG) << COND(1 == 1) << "LOG(DEBUG) will be logged\n";
	LOG(DEBUG) << COND(1 == 2) << "LOG(DEBUG) will not be logged\n";

	/// Colors :-)
	LOG(FATAL) << "LOG(FATAL) " << AixLog::Color::red << "red" << AixLog::Color::none << ", default color\n";
	LOG(FATAL) << "LOG(FATAL) " << COLOR(red) << "red" << COLOR(none) << ", default color (using macros)\n";
	LOG(FATAL) << "LOG(FATAL) " << AixLog::TextColor(AixLog::Color::yellow, AixLog::Color::blue) << "yellow on blue background" << AixLog::Color::none << ", default color\n";
	LOG(FATAL) << "LOG(FATAL) " << COLOR(yellow, blue) << "yellow on blue background" << COLOR(none) << ", default color (using macros)\n";
}
```
