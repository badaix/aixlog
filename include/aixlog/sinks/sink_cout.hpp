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

#include <iostream>

namespace aixlog
{
namespace sinks
{

/**
 * @brief
 * Formatted logging to cout
 */
struct SinkCout : public SinkFormat
{
    SinkCout(const Filter& filter, const std::string& format = "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag_func)") : SinkFormat(filter, format)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        do_log(std::cout, metadata, message);
    }
};


} // namespace sinks
} // namespace aixlog
