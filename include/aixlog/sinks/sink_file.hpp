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

#include <aixlog/sinks/sink_format.hpp>

#include <fstream>


namespace aixlog
{
namespace sinks
{

/**
 * @brief
 * Formatted logging to file
 */
struct SinkFile : public SinkFormat
{
    SinkFile(const Filter& filter, const std::string& filename, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)")
        : SinkFormat(filter, format)
    {
        ofs.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    }

    ~SinkFile() override
    {
        ofs.close();
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        do_log(ofs, metadata, message);
    }

protected:
    mutable std::ofstream ofs;
};


} // namespace sinks
} // namespace aixlog
