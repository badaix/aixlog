# aixlog

C++ logging library

## Features
* Single header file implementation. Simply include and use it!
* Use ostream operator `<<`
* Native support for various platforms (through LogSinks)
  * Linux, Unix: Syslog
  * macOS: Unified logging (os_log)
  * Android: Android Log
  * Windows: OutputDebugString
* Several LogSinks:
  * cout
  * cerr
  * LogSink with custom callback function
  * Easy to add more...
* Manipulators for
  * Different log levels: `LOG(LOG_EMERG) << "some message"`
  * Conditional logging: `LOG(LOG_INFO) << COND(false) << "will not be logged\n"`
  * Support for tags: `LOG(LOG_INFO, "my tag") << "some message"` or `LOG(LOG_INFO) << TAG("my tag") << "some message"`
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
			/// Log everything (prio debug and above, special and normal logs) to OutputDebugString
			make_shared<LogSinkOutputDebugString>(LogPriority::debug, LogSink::Type::all),
			/// Log everything (prio debug and above, special and normal logs) to os_log
			make_shared<LogSinkUnifiedLogging>(LogPriority::debug, LogSink::Type::all),
			/// Log special logs to syslog
			make_shared<LogSinkSyslog>("test", LogPriority::debug, LogSink::Type::special),
			/// Callback log sink with cout logging in a lambda function
			make_shared<LogSinkCallback>(LogPriority::debug, LogSink::Type::all, 
				[](const time_point_sys_clock& timestamp, LogPriority priority, LogType type, const Tag& tag, const std::string& message)
				{
					cout << "Callback:\n\tmsg:  " << message << "\n\ttag:  " << tag.tag << "\n\tprio: " << Log::toString(priority) << " (" << (int)priority << ")\n\ttype: " << (type == LogType::normal?"normal":"special") << "\n";
				}
			)
		}
	);

	/// Log with info prio
	LOG(LOG_INFO) << "LOG(LOG_INFO)\n";
	/// Log with info prio and tag
	LOG(LOG_INFO, "guten tag") << "LOG(LOG_INFO, \"guten tag\")\n";
	/// Log with info prio and explicit tag (same result as above)
	LOG(LOG_INFO) << TAG("guten tag") << "LOGI << TAG(\"guten tag\")\n";
	/// Log with info prio
	LOGI << "LOGI\n";
	/// Log with info prio and explicit tag
	LOGI << TAG("guten tag") << "LOGI << TAG(\"guten tag\")\n";
	/// Log "special" with info prio
	SLOG(LOG_INFO) << "SLOG(LOG_INFO)\n";
	/// Log "special" with info prio
	SLOGI << "SLOGI\n";
	/// Log "special" with info prio
	LOGI << LogType::special << "LOGI << LogType::special\n";
	/// Log "special" with info prio and explicit tag
	LOGI << LogType::special << TAG("guten tag") << "LOGI << LogType::special << TAG(\"guten tag\")\n";

	/// Different log priorities
	LOG(LOG_EMERG) << "LOG(LOG_EMERG)\nLOG(LOG_EMERG) Second line\n";
	LOG(LOG_EMERG) << TAG("hello") << "LOG(LOG_EMERG) << TAG(\"hello\") no line break";
	LOG(LOG_EMERG) << "LOG(LOG_EMERG) 2 no line break";
	LOG(LOG_ALERT) << "LOG(LOG_ALERT): change in loglevel will add a line break";
	LOG(LOG_ERR) << "LOG(LOG_ERR)";
	LOG(LOG_INFO) << TAG("my tag") << "LOG(LOG_INFO) << TAG(\"my tag\")n";
	LOG(LOG_NOTICE) << "LOG(LOG_NOTICE)\n";
	LOG(LOG_INFO) << "LOG(LOG_INFO)\n";
	LOG(LOG_DEBUG) << "LOG(LOG_DEBUG)\n";

	/// Conditional logging
	LOGD << COND(1 == 1) << "LOGD will be logged\n";
	LOGD << COND(1 == 2) << "LOGD will not be logged\n";

	/// Colors :-)
	LOG(LOG_CRIT) << "LOG(LOG_CRIT) " << LogColor::red << "red" << LogColor::none << " default color\n";
	LOG(LOG_CRIT) << "LOG(LOG_CRIT) " << Color(LogColor::yellow, LogColor::blue) << "yellow on blue background" << LogColor::none << " default color\n";
}
```
