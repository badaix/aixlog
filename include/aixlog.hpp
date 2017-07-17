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

#ifndef AIX_LOG_HPP
#define AIX_LOG_HPP

#include <syslog.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#ifdef ANDROID
#include <android/log.h>
#endif


#define LOG(P) std::clog << (LogPriority)P << kNoSyslog
#define SLOG(P) std::clog << (LogPriority)P << kSyslog
#define LOGD LOG(kLogDebug)
#define LOGI LOG(kLogInfo)
#define LOGN LOG(kLogNotice)
#define LOGW LOG(kLogWarning)
#define LOGE LOG(kLogErr)
#define LOGC LOG(kLogCrit)
#define LOGA LOG(kLogAlert)


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


struct LogSink
{
	enum LogSinkType
	{
		kTypeLog = 0,
		kTypeSysLog = 1,
		kTypeAllLog = 2
	};

	LogSink(LogPriority priority = kLogDebug) : priority(priority), sink_type_(kTypeAllLog)
	{
	}

	virtual ~LogSink()
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const = 0;
	virtual LogSinkType get_type() const
	{
		return sink_type_;
	}

	LogPriority priority;
	LogSinkType sink_type_;
};


typedef std::shared_ptr<LogSink> log_sink_ptr;


class Log : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	static Log& instance()
	{
		static Log instance_;
		return instance_;
	}

	/// Without "init" every LOG(X) will simply go to clog
	static void init(const std::vector<log_sink_ptr> log_sinks = {})
	{
		for (auto sink: log_sinks)
			Log::instance().add_logsink(sink);

		std::clog.rdbuf(&Log::instance());
	}

	void add_logsink(log_sink_ptr sink)
	{
		logSinks.push_back(sink);
	}

	void remove_logsink(log_sink_ptr sink)
	{
		logSinks.erase(std::remove(logSinks.begin(), logSinks.end(), sink), logSinks.end());
	}

	static std::string toString(LogPriority logPriority)
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


protected:
	Log() :	syslog_(kNoSyslog)
	{
	}

	int sync()
	{
		if (!buffer_.str().empty())
		{
			auto now = std::chrono::system_clock::now();
			for (const auto sink: logSinks)
			{
				if (
						(sink->get_type() == LogSink::kTypeAllLog) ||
						((syslog_ == kSyslog) && (sink->get_type() == LogSink::kTypeSysLog)) ||
						((syslog_ == kNoSyslog) && (sink->get_type() == LogSink::kTypeLog))
				)
					if (priority_ <= sink->priority)
						sink->log(now, priority_, buffer_.str());
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
/*		if (
				(priority_ > loglevel_) && 
				((syslog_ == kNoSyslog) || !syslog_enabled_) // || (syslogpriority_ > loglevel_))
		)
			return c;
*/		if (c != EOF)
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

	std::stringstream buffer_;
	LogPriority priority_;
	SysLog syslog_;
	std::vector<log_sink_ptr> logSinks;
};




struct LogSinkWithTimestamp : public LogSink
{
	LogSinkWithTimestamp(LogPriority priority, const std::string& timestamp_format = "%Y-%m-%d %H-%M-%S") : 
		LogSink(priority), 
		timestamp_format_(timestamp_format)
	{
	}

	virtual void set_timestamp_format(const std::string& format)
	{
		timestamp_format_ = format;
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const = 0;

	virtual LogSinkType get_type() const
	{
		return kTypeAllLog;
	}

protected:
	
	virtual void do_log(std::ostream& stream, const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const
	{
		std::string prio = "[" + Log::toString(priority) + "]";
		prio.resize(6 + 2, ' ');
		if (!timestamp_format_.empty())
			stream << Timestamp(timestamp, timestamp_format_) << " ";
		stream << prio << message << std::endl;
	}

	/// strftime format + proprietary "#ms" for milliseconds
	std::string Timestamp(const std::chrono::time_point<std::chrono::system_clock>& timestamp, const std::string& format) const
	{
		std::time_t now_c = std::chrono::system_clock::to_time_t(timestamp);
		struct::tm now_tm = *std::localtime(&now_c);

		char buffer[256];
		strftime(buffer, sizeof buffer, format.c_str(), &now_tm);
		std::string result = buffer;
		size_t ms_pos = format.find("#ms");
		if (ms_pos != std::string::npos)
		{
			int ms_part = std::chrono::time_point_cast<std::chrono::milliseconds>(timestamp).time_since_epoch().count() % 1000;
			std::string ms = std::to_string(ms_part);
			while (ms.size() < 3)
				ms = '0' + ms;
			result.replace(ms_pos + 2, 3, ms);
		}
		return result;
	}

	std::string timestamp_format_;
};



struct LogSinkCout : public LogSinkWithTimestamp
{
	LogSinkCout(LogPriority priority, const std::string& timestamp_format = "%Y-%m-%d %H-%M-%S") :
		LogSinkWithTimestamp(priority, timestamp_format)
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const
	{
		if (priority <= this->priority)
			do_log(std::cout, timestamp, priority, message);
	}
};



struct LogSinkCerr : public LogSinkWithTimestamp
{
	LogSinkCerr(LogPriority priority, const std::string& timestamp_format = "%Y-%m-%d %H-%M-%S") :
		LogSinkWithTimestamp(priority, timestamp_format)
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const
	{
		if (priority <= this->priority)
			do_log(std::cerr, timestamp, priority, message);
	}
};



struct LogSinkSyslog : public LogSink
{
	LogSinkSyslog(const char* ident) : LogSink(kLogDebug)
	{
		openlog(ident, LOG_PID, LOG_USER);
	}

	virtual ~LogSinkSyslog()
	{
		closelog();
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const
	{
		syslog((int)priority, "%s", message.c_str());
	}

	virtual LogSinkType get_type() const
	{
		return kTypeSysLog;
	}
};



struct LogSinkAndroid : public LogSink
{
	LogSinkAndroid(LogPriority priority, const std::string& default_tag = "") : LogSink(priority), default_tag_(default_tag)
	{
	}

#ifdef ANDROID
	android_LogPriority get_android_prio(LogPriority priority) const
	{
		switch (priority)
		{
			case kLogEmerg:
			case kLogAlert:
			case kLogCrit:
				return ANDROID_LOG_FATAL;
			case kLogErr:
				return ANDROID_LOG_ERROR;
			case kLogWarning:
				return ANDROID_LOG_WARN;
			case kLogNotice:
				return ANDROID_LOG_DEFAULT;
			case kLogInfo:
				return ANDROID_LOG_INFO;
			case kLogDebug:
				return ANDROID_LOG_DEBUG;
			default: 
				return ANDROID_LOG_UNKNOWN;
		}
	}
#endif

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const std::string& message) const
	{
#ifdef ANDROID
		__android_log_write(get_android_prio(priority), default_tag_.c_str(), message.c_str());
#endif
	}

	virtual LogSinkType get_type() const
	{
		return kTypeAllLog;
	}

protected:
	std::string default_tag_;
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



#endif /// AIX_LOG_HPP


