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

#ifdef _WIN32

#include <aixlog/sinks/sink_base.hpp>

namespace aixlog
{
namespace sinks
{

/**
 * @brief
 * Windows: Logging to OutputDebugString
 *
 * Not tested due to unavailability of Windows
 */
struct SinkOutputDebugString : public sinks::Sink
{
    SinkOutputDebugString(const Filter& filter) : Sink(filter)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
#ifdef UNICODE
        std::wstring wide = std::wstring(message.begin(), message.end());
        OutputDebugString(wide.c_str());
#else
        OutputDebugString(message.c_str());
#endif
    }
};

} // namespace sinks
} // namespace aixlog

#endif
