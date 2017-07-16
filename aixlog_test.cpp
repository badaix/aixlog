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


#include "aixlog.h"

using namespace std;



int main(int argc, char** argv)
{
	Log::init(cout, kLogWarning);
	Log::instance().enable_syslog("test");
	LOG(LOG_EMERG) << "Log emerg\nSecond line\n";
	LOG(LOG_EMERG) << "Log emerg 2";
	LOG(LOG_EMERG) << "Log Second line 2";
	//LOG(LOG_ALERT) << "Log alert";
	//LOG(LOG_CRIT) << "Log crit";
	//LOG(LOG_ERR) << "Log err";
	LOG(LOG_INFO) << "Log warning\n";
	LOG(LOG_NOTICE) << "Log notice\n";
	LOG(LOG_INFO) << "Log info\n";
	LOG(LOG_DEBUG) << "Log debug\n";
	cout << "cout\n";
	cerr << "cerr\n";

	//Log::init(cerr, kLogInfo);
	//syslog(1, "%s", "Test1");
	SLOG(LOG_EMERG) << "Log emerg\nSecond line\n";
	SLOG(LOG_EMERG) << "Log emerg 2";
	SLOG(LOG_EMERG) << "Log Second line 2";
	SLOG(LOG_ALERT) << "Log alert\n";
	SLOG(LOG_CRIT) << "Log crit\n";
	//Log::instance().disable_syslog();
	SLOG(LOG_ERR) << "Log err\n";
	SLOG(LOG_WARNING) << "Log warning\n";
	SLOG(LOG_NOTICE) << "Log notice\n";
	SLOG(LOG_INFO) << "Log info\n";
	SLOG(LOG_DEBUG) << "Log debug\n";

	LOG(LOG_ALERT) << "Log alert\n";
	Log::instance().set_loglevel(kLogDebug);
	LOGD << "Debug\n";
	LOGI << "Info\n";
	LOGE << "Error\n";
}

