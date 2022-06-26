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

#include <functional>

namespace aixlog
{
namespace sinks
{

/**
 * @brief
 * Forward log messages to a callback function
 *
 * Pass the callback function to the c'tor.
 * This can be any function that matches the signature of "callback_fun"
 * Might also be a lambda function
 */
struct SinkCallback : public sinks::Sink
{
    using callback_fun = std::function<void(const Metadata& metadata, const std::string& message)>;

    SinkCallback(const Filter& filter, callback_fun callback) : Sink(filter), callback_(callback)
    {
    }

    void log(const Metadata& metadata, const std::string& message) override
    {
        if (callback_)
            callback_(metadata, message);
    }

private:
    callback_fun callback_;
};


} // namespace sinks
} // namespace aixlog
