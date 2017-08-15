/***
      __   __  _  _  __     __    ___ 
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/
    version 0.10.0
    https://github.com/badaix/aixlog

    This file is part of aixlog
    Copyright (C) 2017  Johannes Pohl
    
    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/

/// inspired by "eater": 
/// https://stackoverflow.com/questions/2638654/redirect-c-stdclog-to-syslog-on-unix

/// TODO: add global log level


#ifndef AIX_LOG_HPP
#define AIX_LOG_HPP

#ifndef _WIN32
#define _HAS_SYSLOG_ 1
#endif

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#ifdef __APPLE__
#include <os/log.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif
#ifdef _HAS_SYSLOG_
#include <syslog.h>
#endif


/// Internal helper defines
#define LOG_WO_TAG(P) std::clog << (AixLog::Severity)P << TAG(__func__)
#define SLOG_WO_TAG(P) std::clog << (AixLog::Severity)P << TAG(__func__) << SPECIAL

#define LOG_TAG(P, T) std::clog << (AixLog::Severity)P << TAG(T)
#define SLOG_TAG(P, T) std::clog << (AixLog::Severity)P << TAG(T) << SPECIAL

#define LOG_X(x,P,T,FUNC, ...)  FUNC
#define SLOG_X(x,P,T,FUNC, ...)  FUNC


/// External logger defines
#define LOG(...) LOG_X(,##__VA_ARGS__, LOG_TAG(__VA_ARGS__), LOG_WO_TAG(__VA_ARGS__))
#define SLOG(...) SLOG_X(,##__VA_ARGS__, SLOG_TAG(__VA_ARGS__), SLOG_WO_TAG(__VA_ARGS__))

#define FUNC __func__
#define TAG AixLog::Tag
#define COND AixLog::Conditional
#define SPECIAL AixLog::Type::special


enum SEVERITY
{
// https://chromium.googlesource.com/chromium/mini_chromium/+/master/base/logging.cc

// Aixlog      Boost      Syslog      Android      macOS      Syslog Desc
//
//                        UNKNOWN
//                        DEFAULT
// trace       trace                 VERBOSE
// debug       debug      DEBUG      DEBUG        DEBUG      debug-level message
// info        info       INFO       INFO         INFO       informational message
// notice                 NOTICE                             normal, but significant, condition
// warning     warning    WARNING    WARN         DEFAULT    warning conditions
// error       error      ERROR      ERROR        ERROR      error conditions
// fatal       fatal      CRIT       FATAL        FAULT      critical conditions
//                        ALERT                              action must be taken immediately
//                        EMERG                              system is unusable

	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	NOTICE = 3,
	WARNING = 4,
	ERROR = 5,
	FATAL = 6
};



namespace AixLog
{

enum class Type
{
	normal,
	special,
	all
};



enum class Severity : std::int8_t
{
	trace   = TRACE,
	debug   = DEBUG,
	info    = INFO,
	notice  = NOTICE,
	warning = WARNING,
	error   = ERROR,
	fatal   = FATAL
};



enum class Color
{
	none = 0,
	black = 1,
	red = 2,
	green = 3,
	yellow = 4,
	blue = 5,
	magenta = 6,
	cyan = 7,
	white = 8
};



struct TextColor
{
	TextColor(Color foreground = Color::none, Color background = Color::none) :
		foreground(foreground),
		background(background)
	{
	}

	Color foreground;
	Color background;
};



struct Conditional
{
	Conditional() : Conditional(true)
	{
	}

	Conditional(bool value) : is_true_(value)
	{
	}

	void set(bool value)
	{
		is_true_ = value;
	}

	bool is_true() const
	{
		return is_true_;
	}

private:
	bool is_true_;
};



struct Tag
{
	Tag(std::nullptr_t) : tag(""), is_null(true)
	{
	}

	Tag() : Tag(nullptr)
	{
	}

	Tag(const std::string& tag) : tag(tag), is_null(false)
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


typedef std::chrono::time_point<std::chrono::system_clock> time_point_sys_clock;

struct Sink
{
	Sink(Severity severity, Type type) : severity(severity), sink_type_(type)
	{
	}

	virtual ~Sink()
	{
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const = 0;
	virtual Type get_type() const
	{
		return sink_type_;
	}

	virtual Sink& set_type(Type sink_type)
	{
		sink_type_ = sink_type;
		return *this;
	}

	Severity severity;

protected:
	Type sink_type_;
};



static std::ostream& operator<< (std::ostream& os, const Severity& log_severity);
static std::ostream& operator<< (std::ostream& os, const Type& log_type);
static std::ostream& operator<< (std::ostream& os, const Tag& tag);
static std::ostream& operator<< (std::ostream& os, const Conditional& conditional);

typedef std::shared_ptr<Sink> log_sink_ptr;


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
		Log::instance().log_sinks_.clear();

		for (auto sink: log_sinks)
			Log::instance().add_logsink(sink);

		std::clog.rdbuf(&Log::instance());
	}

	void add_logsink(log_sink_ptr sink)
	{
		log_sinks_.push_back(sink);
	}

	void remove_logsink(log_sink_ptr sink)
	{
		log_sinks_.erase(std::remove(log_sinks_.begin(), log_sinks_.end(), sink), log_sinks_.end());
	}

	static std::string toString(Severity logSeverity)
	{
		switch (logSeverity)
		{
			case Severity::trace:
				return "Trace";
			case Severity::debug:
				return "Debug";
			case Severity::info:
				return "Info";
			case Severity::notice:
				return "Notice";
			case Severity::warning:
				return "Warn";
			case Severity::error:
				return "Err";
			case Severity::fatal:
				return "Fatal";
			default:
				std::stringstream ss;
				ss << logSeverity;
				return ss.str();
		}
	}


protected:
	Log() :	type_(Type::normal)
	{
	}

	int sync()
	{
		if (!buffer_.str().empty())
		{
			auto now = std::chrono::system_clock::now();
			if (conditional_.is_true())
			{
				for (const auto sink: log_sinks_)
				{
					if (
							(type_ == Type::all) ||
							(sink->get_type() == Type::all) ||
							((type_ == Type::special) && (sink->get_type() == Type::special)) ||
							((type_ == Type::normal) && (sink->get_type() == Type::normal))
					)
						if (severity_ >= sink->severity)
							sink->log(now, severity_, type_, tag_, buffer_.str());
				}
			}
			buffer_.str("");
			buffer_.clear();
			//severity_ = debug; // default to debug for each message
			//type_ = kNormal;
			type_ = Type::normal;
			tag_ = nullptr;
			conditional_.set(true);
		}
		return 0;
	}

	int overflow(int c)
	{
/*		if (
				(severity_ > loglevel_) && 
				((type_ == kNormal) || !syslog_enabled_) // || (syslogseverity_ > loglevel_))
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
			sync();
		}
		return c;
	}


private:
	friend std::ostream& operator<< (std::ostream& os, const Severity& log_severity);
	friend std::ostream& operator<< (std::ostream& os, const Type& log_type);
	friend std::ostream& operator<< (std::ostream& os, const Tag& tag);
	friend std::ostream& operator<< (std::ostream& os, const Conditional& conditional);

	std::stringstream buffer_;
	Severity severity_;
	Type type_;
	Tag tag_;
	Conditional conditional_;
	std::vector<log_sink_ptr> log_sinks_;
};



struct SinkFormat : public Sink
{
	SinkFormat(Severity severity, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S [#severity] (#tag)") : // #logline") : 
		Sink(severity, type), 
		format_(format)
	{
	}

	virtual void set_format(const std::string& format)
	{
		format_ = format;
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const = 0;


protected:
	/// strftime format + proprietary "#ms" for milliseconds
	virtual void do_log(std::ostream& stream, const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
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
			char ms_str[4];
			sprintf(ms_str, "%03d", ms_part);
			result.replace(pos, 3, ms_str);
		}

		pos = result.find("#severity");
		if (pos != std::string::npos)
			result.replace(pos, 9, Log::toString(severity));


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
			if (result.empty() || (result.back() == ' '))
				stream << message << std::endl;
			else
				stream << result << " " << message << std::endl;
		}
	}

	std::string format_;
};



struct SinkCout : public SinkFormat
{
	SinkCout(Severity severity, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag)") : // #logline") :
		SinkFormat(severity, type, format)
	{
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
		if (severity >= this->severity)
			do_log(std::cout, timestamp, severity, type, tag, message);
	}
};



struct SinkCerr : public SinkFormat
{
	SinkCerr(Severity severity, Type type, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag)") : // #logline") :
		SinkFormat(severity, type, format)
	{
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
		if (severity >= this->severity)
			do_log(std::cerr, timestamp, severity, type, tag, message);
	}
};



/// Not tested due to unavailability of Windows
struct SinkOutputDebugString : Sink
{
	SinkOutputDebugString(Severity severity, Type type = Type::all, const std::string& default_tag = "") : Sink(severity, type)
	{
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
#ifdef _WIN32
		OutputDebugString(message.c_str());
#endif
	}
};



struct SinkUnifiedLogging : Sink
{
	SinkUnifiedLogging(Severity severity, Type type = Type::all) : Sink(severity, type)
	{
	}

#ifdef __APPLE__
	os_log_type_t get_os_log_type(Severity severity) const
	{
		// https://developer.apple.com/documentation/os/os_log_type_t?language=objc
		switch (severity)
		{
			case Severity::trace:
			case Severity::debug:
				return OS_LOG_TYPE_DEBUG;
			case Severity::info:
			case Severity::notice:
				return OS_LOG_TYPE_INFO;
			case Severity::warning:
				return OS_LOG_TYPE_DEFAULT;
			case Severity::error:
				return OS_LOG_TYPE_ERROR;
			case Severity::fatal:
				return OS_LOG_TYPE_FAULT;
			default: 
				return OS_LOG_TYPE_DEFAULT;
		}
	}
#endif

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
#ifdef __APPLE__
		os_log_with_type(OS_LOG_DEFAULT, get_os_log_type(severity), "%{public}s", message.c_str());
#endif
	}
};



struct SinkSyslog : public Sink
{
	SinkSyslog(const char* ident, Severity severity, Type type) : Sink(severity, type)
	{
#ifdef _HAS_SYSLOG_
		openlog(ident, LOG_PID, LOG_USER);
#endif
	}

	virtual ~SinkSyslog()
	{
#ifdef _HAS_SYSLOG_
		closelog();
#endif
	}

#ifdef _HAS_SYSLOG_
	int get_syslog_priority(Severity severity) const
	{
		// http://unix.superglobalmegacorp.com/Net2/newsrc/sys/syslog.h.html
		switch (severity)
		{
			case Severity::trace:
			case Severity::debug:
				return LOG_DEBUG;
			case Severity::info:
				return LOG_INFO;
			case Severity::notice:
				return LOG_NOTICE;
			case Severity::warning:
				return LOG_WARNING;
			case Severity::error:
				return LOG_ERR;
			case Severity::fatal:
				return LOG_CRIT;
			default: 
				return LOG_INFO;
		}
	}
#endif


	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
#ifdef _HAS_SYSLOG_
		syslog(get_syslog_priority(severity), "%s", message.c_str());
#endif
	}
};



struct SinkAndroid : public Sink
{
	SinkAndroid(const std::string& ident, Severity severity, Type type = Type::all) : Sink(severity, type), ident_(ident)
	{
	}

#ifdef __ANDROID__
	android_LogPriority get_android_prio(Severity severity) const
	{
		// https://developer.android.com/ndk/reference/log_8h.html
		switch (severity)
		{
			case Severity::trace:
				return ANDROID_LOG_VERBOSE;
			case Severity::debug:
				return ANDROID_LOG_DEBUG;
			case Severity::info:
			case Severity::notice:
				return ANDROID_LOG_INFO;
			case Severity::warning:
				return ANDROID_LOG_WARN;
			case Severity::error:
				return ANDROID_LOG_ERROR;
			case Severity::fatal:
				return ANDROID_LOG_FATAL;
			default: 
				return ANDROID_LOG_UNKNOWN;
		}
	}
#endif

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
#ifdef __ANDROID__
		std::string log_tag;// = default_tag_;
		if (tag)
		{
			if (!ident_.empty())
				log_tag = ident_ + "." + tag.tag;
			else
				log_tag = tag.tag;
		}
		else
			log_tag = ident_;

		__android_log_write(get_android_prio(severity), log_tag.c_str(), message.c_str());
#endif
	}

protected:
	std::string ident_;
};



/// Not tested due to unavailability of Windows
struct SinkEventLog : public Sink
{
	SinkEventLog(const std::string& ident, Severity severity, Type type = Type::all) : Sink(severity, type)
	{
#ifdef _WIN32
		event_log = RegisterEventSource(NULL, ident.c_str());
#endif
	}

#ifdef _WIN32
	WORD get_type(Severity severity) const
	{
		// https://msdn.microsoft.com/de-de/library/windows/desktop/aa363679(v=vs.85).aspx
		switch (severity)
		{
			case Severity::trace:
			case Severity::debug:
				return EVENTLOG_INFORMATION_TYPE;
			case Severity::info:
			case Severity::notice:
				return EVENTLOG_SUCCESS;
			case Severity::warning:
				return EVENTLOG_WARNING_TYPE;
			case Severity::error:
			case Severity::fatal:
				return EVENTLOG_ERROR_TYPE;
			default: 
				return EVENTLOG_INFORMATION_TYPE;
		}
	}
#endif

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
#ifdef _WIN32
		ReportEvent(event_log, get_type(severity), 0, 0, NULL, 1, 0, &message.c_str(), NULL);
#endif
	}

protected:
#ifdef _WIN32
	HANDLE event_log;
#endif
};



struct SinkNative : public Sink
{
	SinkNative(const std::string& ident, Severity severity, Type type = Type::all) : 
		Sink(severity, type), 
		log_sink_(nullptr), 
		ident_(ident)
	{
#ifdef __ANDROID__
		log_sink_ = std::make_shared<SinkAndroid>(ident_, severity, type);
#elif __APPLE__
		log_sink_ = std::make_shared<SinkUnifiedLogging>(severity, type);
#elif _WIN32
		log_sink_ = std::make_shared<SinkEventLog>(severity, type);
#elif _HAS_SYSLOG_
		log_sink_ = std::make_shared<SinkSyslog>(ident_.c_str(), severity, type);
#else
		/// will not throw or something. Use "get_logger()" to check for success
		log_sink_ = nullptr;
#endif
	}

	virtual log_sink_ptr get_logger()
	{
		return log_sink_;
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
		if (log_sink_)
			log_sink_->log(timestamp, severity, type, tag, message);
	}

protected:
	log_sink_ptr log_sink_;
	std::string ident_;
};



struct SinkCallback : public Sink
{
	typedef std::function<void(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message)> callback_fun;

	SinkCallback(Severity severity, Type type, callback_fun callback) : Sink(severity, type), callback(callback)
	{
	}

	virtual void log(const time_point_sys_clock& timestamp, const Severity& severity, const Type& type, const Tag& tag, const std::string& message) const
	{
		if (callback && (severity >= this->severity))
			callback(timestamp, severity, type, tag, message);
	}

private:
	callback_fun callback;
};



static std::ostream& operator<< (std::ostream& os, const Severity& log_severity)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log && (log->severity_ != log_severity))
	{
		log->sync();
		log->severity_ = log_severity;
	}
	return os;
}



static std::ostream& operator<< (std::ostream& os, const Type& log_type)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log)
		log->type_ = log_type;
	return os;
}



static std::ostream& operator<< (std::ostream& os, const Tag& tag)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log)
		log->tag_ = tag;
	return os;
}



static std::ostream& operator<< (std::ostream& os, const Conditional& conditional)
{
	Log* log = dynamic_cast<Log*>(os.rdbuf());
	if (log)
		log->conditional_.set(conditional.is_true());
	return os;
}



static std::ostream& operator<< (std::ostream& os, const TextColor& log_color)
{
	os << "\033[";
	if ((log_color.foreground == Color::none) && (log_color.background == Color::none))
		os << "0"; // reset colors if no params

	if (log_color.foreground != Color::none) 
	{
		os << 29 + (int)log_color.foreground;
		if (log_color.background != Color::none) 
			os << ";";
	}
	if (log_color.background != Color::none) 
		os << 39 + (int)log_color.background;
	os << "m";

	return os;
}



static std::ostream& operator<< (std::ostream& os, const Color& color)
{
	os << TextColor(color);
	return os;
}


}


#endif /// AIX_LOG_HPP


