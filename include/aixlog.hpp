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


#define LOG(P) std::clog << (LogPriority)P << LogType::normal
#define SLOG(P) std::clog << (LogPriority)P << LogType::special
#define LOGD LOG(LogPriority::debug)
#define LOGI LOG(LogPriority::info)
#define LOGN LOG(LogPriority::notice)
#define LOGW LOG(LogPriority::warning)
#define LOGE LOG(LogPriority::error)
#define LOGC LOG(LogPriority::critical)
#define LOGA LOG(LogPriority::alert)


enum class LogType
{
	normal,
	special
};


enum class LogPriority : std::int8_t
{
	emerg   = LOG_EMERG,   // 0 system is unusable
	alert   = LOG_ALERT,   // 1 action must be taken immediately
	critical= LOG_CRIT,    // 2 critical conditions
	error   = LOG_ERR,     // 3 error conditions
	warning = LOG_WARNING, // 4 warning conditions
	notice  = LOG_NOTICE,  // 5 normal, but significant, condition
	info    = LOG_INFO,    // 6 informational message
	debug   = LOG_DEBUG    // 7 debug-level message
};



struct TAG
{
	TAG(std::nullptr_t) : tag(""), is_null(true)
	{
	}

	TAG() : TAG(nullptr)
	{
	}

	TAG(const std::string& tag) : tag(tag), is_null(false)
	{
	}

	virtual explicit operator bool() const
	{
		return !is_null;
	}

	std::string tag;

private:
	bool is_null;
};



struct LogSink
{
	enum class Type
	{
		normal,
		special,
		all
	};

	LogSink(LogPriority priority, Type type) : priority(priority), sink_type_(type)
	{
	}

	virtual ~LogSink()
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const = 0;
	virtual Type get_type() const
	{
		return sink_type_;
	}

	virtual LogSink& set_type(Type sink_type)
	{
		sink_type_ = sink_type;
		return *this;
	}

	LogPriority priority;

protected:
	Type sink_type_;
};



std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority);
std::ostream& operator<< (std::ostream& os, const LogType& log_type);
std::ostream& operator<< (std::ostream& os, const TAG& tag);

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
			case LogPriority::emerg:
				return "Emerg";
			case LogPriority::alert:
				return "Alert";
			case LogPriority::critical:
				return "Crit";
			case LogPriority::error:
				return "Err";
			case LogPriority::warning:
				return "Warn";
			case LogPriority::notice:
				return "Notice";
			case LogPriority::info:
				return "Info";
			case LogPriority::debug:
				return "Debug";
			default:
				std::stringstream ss;
				ss << logPriority;
				return ss.str();
		}
	}


protected:
	Log() :	type_(LogType::normal)
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
						(sink->get_type() == LogSink::Type::all) ||
						((type_ == LogType::special) && (sink->get_type() == LogSink::Type::special)) ||
						((type_ == LogType::normal) && (sink->get_type() == LogSink::Type::normal))
				)
					if (priority_ <= sink->priority)
						sink->log(now, priority_, tag_, buffer_.str());
			}
			buffer_.str("");
			buffer_.clear();
			//priority_ = debug; // default to debug for each message
			//type_ = kNormal;
			tag_ = nullptr;
		}
		return 0;
	}

	int overflow(int c)
	{
/*		if (
				(priority_ > loglevel_) && 
				((type_ == kNormal) || !syslog_enabled_) // || (syslogpriority_ > loglevel_))
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
	friend std::ostream& operator<< (std::ostream& os, const LogType& log_type);
	friend std::ostream& operator<< (std::ostream& os, const TAG& tag);

	std::stringstream buffer_;
	LogPriority priority_;
	LogType type_;
	TAG tag_;
	std::vector<log_sink_ptr> logSinks;
};



struct LogSinkFormat : public LogSink
{
	LogSinkFormat(LogPriority priority, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S [#prio] (#tag)") : // #logline") : 
		LogSink(priority, type), 
		format_(format)
	{
		sink_type_ = Type::all;
	}

	virtual void set_format(const std::string& format)
	{
		format_ = format;
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const = 0;


protected:
	
	/// strftime format + proprietary "#ms" for milliseconds
	virtual void do_log(std::ostream& stream, const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const
	{
		std::time_t now_c = std::chrono::system_clock::to_time_t(timestamp);
		struct::tm now_tm = *std::localtime(&now_c);

		char buffer[256];
		strftime(buffer, sizeof buffer, format_.c_str(), &now_tm);
		std::string result = buffer;
		size_t pos = result.find("#ms");
		if (pos != std::string::npos)
		{
			int ms_part = std::chrono::time_point_cast<std::chrono::milliseconds>(timestamp).time_since_epoch().count() % 1000;
			std::string ms = std::to_string(ms_part);
			while (ms.size() < 3)
				ms = '0' + ms;
			result.replace(pos, 3, ms);
		}

		pos = result.find("#prio");
		if (pos != std::string::npos)
			result.replace(pos, 5, Log::toString(priority));


		pos = result.find("#tag");
		if (pos != std::string::npos)
			result.replace(pos, 4, tag?tag.tag:"log");

		pos = result.find("#logline");
		if (pos != std::string::npos)
		{
			result.replace(pos, 8, message);
			stream << result << std::endl;
		}
		else
		{
			if (result.empty())
				stream << message << std::endl;
			else
				stream << result << " " << message << std::endl;
		}
	}

	std::string format_;
};



struct LogSinkCout : public LogSinkFormat
{
	LogSinkCout(LogPriority priority, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#prio] (#tag)") : // #logline") :
		LogSinkFormat(priority, type, format)
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const
	{
		if (priority <= this->priority)
			do_log(std::cout, timestamp, priority, tag, message);
	}
};



struct LogSinkCerr : public LogSinkFormat
{
	LogSinkCerr(LogPriority priority, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#prio] (#tag)") : // #logline") :
		LogSinkFormat(priority, type, format)
	{
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const
	{
		if (priority <= this->priority)
			do_log(std::cerr, timestamp, priority, tag, message);
	}
};



struct LogSinkSyslog : public LogSink
{
	LogSinkSyslog(const char* ident, LogPriority priority, Type type) : LogSink(priority, type)
	{
		openlog(ident, LOG_PID, LOG_USER);
	}

	virtual ~LogSinkSyslog()
	{
		closelog();
	}

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const
	{
		syslog((int)priority, "%s", message.c_str());
	}
};



struct LogSinkAndroid : public LogSink
{
	LogSinkAndroid(LogPriority priority, Type type = Type::all, const std::string& default_tag = "") : LogSink(priority, type), default_tag_(default_tag)
	{
	}

#ifdef ANDROID
	android_LogPriority get_android_prio(LogPriority priority) const
	{
		switch (priority)
		{
			case emerg:
			case alert:
			case critical:
				return ANDROID_LOG_FATAL;
			case error:
				return ANDROID_LOG_ERROR;
			case warning:
				return ANDROID_LOG_WARN;
			case notice:
				return ANDROID_LOG_DEFAULT;
			case info:
				return ANDROID_LOG_INFO;
			case debug:
				return ANDROID_LOG_DEBUG;
			default: 
				return ANDROID_LOG_UNKNOWN;
		}
	}
#endif

	virtual void log(const std::chrono::time_point<std::chrono::system_clock>& timestamp, LogPriority priority, const TAG& tag, const std::string& message) const
	{
#ifdef ANDROID
		__android_log_write(get_android_prio(priority), tag?tag.tag.c_str():default_tag_.c_str(), message.c_str());
#endif
	}

protected:
	std::string default_tag_;
};



std::ostream& operator<< (std::ostream& os, const LogPriority& log_priority)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log && (log->priority_ != log_priority))
	{
		log->sync();
		log->priority_ = log_priority;
	}
	return os;
}



std::ostream& operator<< (std::ostream& os, const LogType& log_type)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log)
		log->type_ = log_type;
	return os;
}



std::ostream& operator<< (std::ostream& os, const TAG& tag)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log)
		log->tag_ = tag;
	return os;
}



#endif /// AIX_LOG_HPP


