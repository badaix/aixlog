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
			make_shared<LogSinkCallback>(LogPriority::debug, LogSink::Type::all, 
				[](const time_point_sys_clock& timestamp, LogPriority priority, LogType type, const Tag& tag, const std::string& message)
				{
					cout << "Callback:\n\tmsg:  " << message << "\n\ttag:  " << tag.tag << "\n\tprio: " << Log::toString(priority) << " (" << (int)priority << ")\n\ttype: " << (type == LogType::normal?"normal":"special") << "\n";
				}
			),
			make_shared<LogSinkCout>(LogPriority::debug, LogSink::Type::normal, "cout: %Y-%m-%d %H-%M-%S.#ms [#prio] (#tag) #logline"),
			make_shared<LogSinkCerr>(LogPriority::error, LogSink::Type::all, "cerr: %Y-%m-%d %H-%M-%S.#ms [#prio] (#tag)"),
			make_shared<LogSinkOutputDebugString>(LogPriority::debug, LogSink::Type::all),
			make_shared<LogSinkUnifiedLogging>(LogPriority::debug, LogSink::Type::all),
			make_shared<LogSinkSyslog>("test", LogPriority::debug, LogSink::Type::special)
		}
	);

	LOG(LOG_INFO, "guten tag") << "LOG(LOG_INFO, \"guten tag\")\n";
	LOG(LOG_INFO) << "LOG(LOG_INFO)\n";
	LOGI << TAG("guten tag") << "LOGI << TAG(\"guten tag\")\n";
	LOGI << "LOGI\n";

	LOG(LOG_EMERG) << "LOG(LOG_EMERG)\nLOG(LOG_EMERG) Second line\n";
	LOG(LOG_EMERG) << TAG("hello") << "LOG(LOG_EMERG) << TAG(\"hello\") no line break";
	LOG(LOG_EMERG) << "LOG(LOG_EMERG) 2 no line break";
	LOG(LOG_ALERT) << "LOG(LOG_ALERT): change in loglevel will add a line break";
	LOG(LOG_CRIT) << "LOG(LOG_CRIT)";
	LOG(LOG_ERR) << "LOG(LOG_ERR)";
	LOG(LOG_INFO) << TAG("my tag") << "LOG(LOG_INFO) << TAG(\"my tag\")n";
	LOG(LOG_NOTICE) << "LOG(LOG_NOTICE)\n";
	LOG(LOG_INFO) << "LOG(LOG_INFO)\n";
	LOG(LOG_DEBUG) << "LOG(LOG_DEBUG)\n";
	cout << "cout\n";
	cerr << "cerr\n";

	SLOG(LOG_EMERG) << "SLOG(LOG_EMERG)\nSLOG(LOG_EMERG) Second line\n";
	SLOG(LOG_EMERG) << "SLOG(LOG_EMERG) 1, ";
	SLOG(LOG_EMERG) << "SLOG(LOG_EMERG) 2\n";
	SLOG(LOG_EMERG) << "SLOG(LOG_EMERG) 1, "
					<< "SLOG(LOG_EMERG) 2\n";
	SLOG(LOG_ALERT) << "SLOG(LOG_ALERT)\n";
	SLOG(LOG_CRIT) << "SLOG(LOG_CRIT)\n";
	SLOG(LOG_ERR) << "SLOG(LOG_ERR)\n";
	SLOG(LOG_WARNING) << "SLOG(LOG_WARNING)\n";
	SLOG(LOG_NOTICE) << "SLOG(LOG_NOTICE)\n";
	SLOG(LOG_INFO) << "SLOG(LOG_INFO)\n";
	SLOG(LOG_DEBUG) << "SLOG(LOG_DEBUG)\n";

	LOG(LOG_ALERT) << "LOG(LOG_ALERT)\n";
	LOGD << "LOGD\n";
	LOGD << COND(1 == 1) << "LOGD will be logged\n";
	LOGD << COND(1 == 2) << "LOGD will not be logged\n";
	LOGI << "LOGI\n";
	LOGE << "LOGE\n";
}
```
