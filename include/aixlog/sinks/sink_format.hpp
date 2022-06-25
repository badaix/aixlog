/***
      __   __  _  _  __     __    ___
     / _\ (  )( \/ )(  )   /  \  / __)
    /    \ )(  )  ( / (_/\(  O )( (_ \
    \_/\_/(__)(_/\_)\____/ \__/  \___/
    version 1.5.0
    https://github.com/badaix/aixlog

    This file is part of aixlog
    Copyright (C) 2017-2021 Johannes Pohl

    This software may be modified and distributed under the terms
    of the MIT license.  See the LICENSE file for details.
***/


#pragma once

#include <aixlog/sinks/sink_base.hpp>

namespace aixlog
{
namespace sinks
{

/**
 * @brief
 * Abstract log sink with support for formatting log message
 *
 * "format" in the c'tor defines a log pattern.
 * For every log message, these placeholders will be substituded:
 * - strftime syntax is used to format the logging time stamp (%Y, %m, %d, ...)
 * - #ms: milliseconds part of the logging time stamp with leading zeros
 * - #severity: log severity
 * - #tag_func: the log tag. If empty, the function
 * - #tag: the log tag
 * - #function: the function
 * - #message: the log message
 */
struct SinkFormat : public sinks::Sink
{
    SinkFormat(const Filter& filter, const std::string& format) : Sink(filter), format_(format)
    {
    }

    virtual void set_format(const std::string& format)
    {
        format_ = format;
    }

    void log(const Metadata& metadata, const std::string& message) override = 0;

protected:
    virtual void do_log(std::ostream& stream, const Metadata& metadata, const std::string& message) const
    {
        std::string result = format_;
        if (metadata.timestamp)
            result = metadata.timestamp.to_string(result);

        size_t pos = result.find("#severity");
        if (pos != std::string::npos)
            result.replace(pos, 9, to_string(metadata.severity));

        pos = result.find("#tag_func");
        if (pos != std::string::npos)
            result.replace(pos, 9, metadata.tag ? metadata.tag.text : (metadata.function ? metadata.function.name : "log"));

        pos = result.find("#tag");
        if (pos != std::string::npos)
            result.replace(pos, 4, metadata.tag ? metadata.tag.text : "");

        pos = result.find("#function");
        if (pos != std::string::npos)
            result.replace(pos, 9, metadata.function ? metadata.function.name : "");

        pos = result.find("#message");
        if (pos != std::string::npos)
        {
            result.replace(pos, 8, message);
            stream << result << std::endl;
        }
        else
        {
            if (result.empty() || (result.back() == ' '))
                stream << result << message << std::endl;
            else
                stream << result << " " << message << std::endl;
        }
    }

    std::string format_;
};

} // namespace sinks
} // namespace aixlog
