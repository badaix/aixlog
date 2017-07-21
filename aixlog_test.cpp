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

	Test test;
	test.do_something("doing something...");
}

