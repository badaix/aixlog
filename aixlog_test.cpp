/***
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
			make_shared<LogSinkCout>(LogPriority::critical, LogSink::Type::all),
			make_shared<LogSinkCout>(LogPriority::debug, LogSink::Type::all, ""),
			make_shared<LogSinkSyslog>("test", LogPriority::debug, LogSink::Type::special)
//			make_shared<LogSinkSyslog>("test")->set_type(LogSink::Type::all)
//			make_shared<LogSinkCerr>(LogPriority::debug),
//			make_shared<LogSinkAndroid>(LogPriority::debug)
		}
	);


	LOG(LOG_EMERG) << "Log emerg\nSecond line\n";
	LOG(LOG_EMERG) << TAG("hallo") << "Log emerg 2";
	LOG(LOG_EMERG) << "Log Second line 2";
	//LOG(LOG_ALERT) << "Log alert";
	//LOG(LOG_CRIT) << "Log crit";
	//LOG(LOG_ERR) << "Log err";
	LOG(LOG_INFO) << TAG("my tag") << "Log warning\n";
	LOG(LOG_NOTICE) << "Log notice\n";
	LOG(LOG_INFO) << "Log info\n";
	LOG(LOG_DEBUG) << "Log debug\n";
	cout << "cout\n";
	cerr << "cerr\n";

	SLOG(LOG_EMERG) << "SysLog emerg\nSecond line\n";
	SLOG(LOG_EMERG) << "SysLog emerg 2";
	SLOG(LOG_EMERG) << "SysLog Second line 2";
	SLOG(LOG_ALERT) << "SysLog alert\n";
	SLOG(LOG_CRIT) << "SysLog crit\n";
	SLOG(LOG_ERR) << "SysLog err\n";
	SLOG(LOG_WARNING) << "SysLog warning\n";
	SLOG(LOG_NOTICE) << "SysLog notice\n";
	SLOG(LOG_INFO) << "SysLog info\n";
	SLOG(LOG_DEBUG) << "SysLog debug\n";

	LOG(LOG_ALERT) << "Log alert\n";
	LOGD << "Debug\n";
	LOGI << "Info\n";
	LOGE << "Error\n";
}

