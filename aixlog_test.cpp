/***
      __   __  _  _  __     __    ___ 
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/

    This file is part of aixlog
    Copyright (C) 2017  Johannes Pohl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/


#include "aixlog.hpp"

using namespace std;


class Test
{
public:
	Test()
	{
	}

	void do_something(const std::string& some_string)
	{
		LOGI << "LOGI: " << some_string << "\n";
	}
};



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

	Test test;
	test.do_something("doing something...");
}

