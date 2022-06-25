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
 * Null log sink
 *
 * Discards all log messages
 */
struct SinkNull : public Sink
{
    SinkNull() : Sink(Filter())
    {
    }

    void log(const Metadata& /*metadata*/, const std::string& /*message*/) override
    {
    }
};


} // namespace sinks
} // namespace aixlog
