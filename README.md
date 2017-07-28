# aixlog

C++ logging library

## Features
* Single header file implementation
  * Simply include and use it!
  * Small code base: adapt it for your needs
  * No dependcies, just vanilla C++11
* Use ostream operator `<<`
  * easy to switch from existing "`cout` logging"
* Fancy name
* Native support for various platforms (through LogSinks)
  * Linux, Unix: Syslog
  * macOS: Unified logging (os_log)
  * Android: Android Log
  * Windows: Event log, OutputDebugString
* Several LogSinks:
  * cout
  * cerr
  * LogSink with custom callback function
    * implement your own log sink in a lambda with a single line of code
  * Easy to add more...
* Manipulators for
  * Different log levels: `LOG(LOG_EMERG) << "some message"`
  * Conditional logging: `LOG(LOG_INFO) << COND(false) << "will not be logged\n"`
  * Support for tags:  
    `LOG(LOG_INFO, "my tag") << "some message"`  
    ...is the same as...  
    `LOG(LOG_INFO) << TAG("my tag") << "some message"`
  * Two different log types "normal" and "special": `LOG(LOG_INFO) << LogType::special << "some message"`
    * special might be used for syslog, while normal is used for console output
    * => Only special tagged messages will go to syslog

## Usage Example
```c++
#include "aixlog.hpp"

using namespace std;

int main(int argc, char** argv)
{
	Log::init(
		{
			/// Log normal (i.e. non-special) logs to LogSinkCout
			make_shared<LogSinkCout>(LogPriority::debug, LogSink::Type::normal, "cout: %Y-%m-%d %H-%M-%S.#ms [#prio] (#tag) #logline"),
			/// Log error and higher prio messages to cerr
			make_shared<LogSinkCerr>(LogPriority::error, LogSink::Type::all, "cerr: %Y-%m-%d %H-%M-%S.#ms [#prio] (#tag)"),
			/// Log special logs to native log (Syslog on Linux, Android Log on Android, EventLog on Windows, Unified logging on Apple)
			make_shared<LogSinkNative>("aixlog", LogPriority::debug, LogSink::Type::special),
			/// Callback log sink with cout logging in a lambda function
			/// Could also do file logging
			make_shared<LogSinkCallback>(LogPriority::debug, LogSink::Type::all, 
				[](const time_point_sys_clock& timestamp, LogPriority priority, LogType type, const Tag& tag, const std::string& message)
				{
					cout << "Callback:\n\tmsg:  " << message << "\n\ttag:  " << tag.tag << "\n\tprio: " << Log::toString(priority) << " (" << (int)priority << ")\n\ttype: " << (type == LogType::normal?"normal":"special") << "\n";
				}
			)
		}
	);

	/// Log with info priority
	LOG(INFO) << "LOG(INFO)\n";
	/// ... with a tag
	LOG(INFO, "guten tag") << "LOG(INFO, \"guten tag\")\n";
	/// ... with an explicit tag (same result as above)
	LOG(INFO) << TAG("guten tag") << "LOGI << TAG(\"guten tag\")\n";
	/// Log "special" with info prio
	SLOG(INFO) << "SLOG(INFO)\n";
	/// Log with explicit "special" type
	LOG(INFO) << LogType::special << "LOGI << LogType::special\n";
	/// ... with explicit "special" type and explicit tag
	LOG(INFO) << LogType::special << TAG("guten tag") << "LOGI << LogType::special << TAG(\"guten tag\")\n";

	/// Different log priorities
	LOG(EMERG) << "LOG(EMERG)\nLOG(EMERG) Second line\n";
	LOG(EMERG) << TAG("hello") << "LOG(EMERG) << TAG(\"hello\") no line break";
	LOG(EMERG) << "LOG(EMERG) 2 no line break";
	LOG(ALERT) << "LOG(ALERT): change in loglevel will add a line break";
	LOG(CRIT) << "LOG(CRIT)";
	LOG(ERROR) << "LOG(ERROR)";
	LOG(WARNING) << "LOG(WARNING)";
	LOG(NOTICE) << "LOG(NOTICE)\n";
	LOG(INFO) << "LOG(INFO)\n";
	LOG(INFO) << TAG("my tag") << "LOG(INFO) << TAG(\"my tag\")n";
	LOG(DEBUG) << "LOG(DEBUG)\n";

	/// Conditional logging
	LOG(DEBUG) << COND(1 == 1) << "LOG(DEBUG) will be logged\n";
	LOG(DEBUG) << COND(1 == 2) << "LOG(DEBUG) will not be logged\n";

	/// Colors :-)
	LOG(CRIT) << "LOG(CRIT) " << Color::red << "red" << Color::none << " default color\n";
	LOG(CRIT) << "LOG(CRIT) " << LogColor(Color::yellow, Color::blue) << "yellow on blue background" << Color::none << " default color\n";
}
```
