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

