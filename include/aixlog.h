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

/// inspired by "eater": 
/// https://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix

#ifndef LOG_H
#define LOG_H

#include <syslog.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <ios>
#include <stdexcept>


#define LOG(P) std::clog << (LogPriority)P << kNoSyslog
#define SLOG(P) std::clog << (LogPriority)P << kSyslog
#define LOGD LOG(kLogDebug)
#define LOGI LOG(kLogInfo)
#define LOGE LOG(kLogErr)


enum SysLog
{
	kNoSyslog = 0,
	kSyslog = 1
};


enum LogPriority
{
	kLogEmerg   = LOG_EMERG,   // 0 system is unusable
	kLogAlert   = LOG_ALERT,   // 1 action must be taken immediately
	kLogCrit    = LOG_CRIT,    // 2 critical conditions
	kLogErr     = LOG_ERR,     // 3 error conditions
	kLogWarning = LOG_WARNING, // 4 warning conditions
	kLogNotice  = LOG_NOTICE,  // 5 normal, but significant, condition
	kLogInfo    = LOG_INFO,    // 6 informational message
	kLogDebug   = LOG_DEBUG    // 7 debug-level message
};


std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
std::ostream& operator<< (std::ostream& os, const SysLog& log_syslog);


class Log : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	static Log& instance()
	{
		static Log instance_;
		return instance_;
	}

	static void init(std::ostream& ostream, LogPriority loglevel, const std::string& timestamp_format = "%Y-%m-%d %H-%M-%S")
	{
		Log::instance().set_timestamp_format(timestamp_format);
		Log::instance().set_ostream(ostream);
		Log::instance().set_loglevel(loglevel);
		std::clog.rdbuf(&Log::instance());
	}

	void set_timestamp_format(const std::string& format)
	{
		timestamp_format_ = format;
	}

	void set_ostream(std::ostream& ostream)
	{
		ostream_ = &ostream;
	}

	void set_loglevel(LogPriority priority)
	{
		loglevel_ = priority;
	}

	void enable_syslog(const char* ident)//, int facility)
	{
		openlog(ident, LOG_PID, LOG_USER);
		syslog_enabled_ = true;
	}

	void disable_syslog()
	{
		syslog_enabled_ = false;
		closelog();
	}


protected:
	Log() :	
		syslog_(kNoSyslog),
		ostream_(&std::cout),
		loglevel_(kLogDebug),
		timestamp_format_("%Y-%m-%d %H-%M-%S"),
		syslog_enabled_(false)
	{
		if (*ostream_ == std::clog)
			throw std::invalid_argument("clog is not allowed as output stream");
	}

	int sync()
	{
		if (!buffer_.str().empty())
		{
			if (
				(syslog_enabled_ && (syslog_ == kSyslog)) || // && (syslogpriority_ <= loglevel_)) || 
				(priority_ <= loglevel_)
			)
			{			
				std::string prio = "[" + toString(priority_) + "]";
				prio.resize(6 + 2, ' ');
				if (priority_ <= loglevel_)
				{
					if (!timestamp_format_.empty())
						*ostream_ << Timestamp() << " ";
					*ostream_ << prio << buffer_.str() << std::endl;//flush;
				}
				if (syslog_enabled_ && (syslog_ == kSyslog)) // && (syslogpriority_ <= loglevel_))
				{
					syslog((int)priority_, "%s", buffer_.str().c_str());
				}
			}
			buffer_.str("");
			buffer_.clear();
			//priority_ = kLogDebug; // default to debug for each message
			//syslog_ = kNoSyslog;
		}
		return 0;
	}

	int overflow(int c)
	{
		if (
				(priority_ > loglevel_) && 
				((syslog_ == kNoSyslog) || !syslog_enabled_) // || (syslogpriority_ > loglevel_))
		)
			return c;
		if (c != EOF)
		{
			if (c == '\n')
				sync();
			else
				buffer_ << static_cast<char>(c);
		}
		else
		{
			std::cout << "EOF\n";
			sync();
		}
		return c;
	}


private:
	friend std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
	friend std::ostream& operator<< (std::ostream& os, const SysLog& log_syslog);

	std::string toString(LogPriority logPriority) const
	{
		switch (logPriority)
		{
			case kLogEmerg:
				return "Emerg";
			case kLogAlert:
				return "Alert";
			case kLogCrit:
				return "Crit";
			case kLogErr:
				return "Err";
			case kLogWarning:
				return "Warn";
			case kLogNotice:
				return "Notice";
			case kLogInfo:
				return "Info";
			case kLogDebug:
				return "Debug";
			default:
				std::stringstream ss;
				ss << logPriority;
				return ss.str();
		}
	}

	std::string Timestamp() const
	{
		struct tm * dt;
		char buffer[30];
		std::time_t t = std::time(nullptr);
		dt = localtime(&t);
		strftime(buffer, sizeof(buffer), timestamp_format_.c_str(), dt);
		return std::string(buffer);
	}

	std::stringstream buffer_;
	LogPriority priority_;
	SysLog syslog_;
	std::ostream* ostream_;
	LogPriority loglevel_;
	std::string timestamp_format_;
	bool syslog_enabled_;
};


std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority)
{
	Log* log = static_cast<Log*>(os.rdbuf());
	if (log->priority_ != log_priority)
	{
		log->sync();
		log->priority_ = log_priority;
	}
	return os;
}



std::ostream& operator<< (std::ostream& os, const SysLog& log_syslog)
{
	static_cast<Log*>(os.rdbuf())->syslog_ = log_syslog;
	return os;
}



#endif


