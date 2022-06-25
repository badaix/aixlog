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

#include <aixlog/metadata.hpp>

#include <map>


namespace aixlog
{

class Filter
{
public:
    Filter()
    {
    }

    Filter(Severity severity)
    {
        add_filter(severity);
    }

    bool match(const Metadata& metadata) const
    {
        if (tag_filter_.empty())
            return true;

        auto iter = tag_filter_.find(metadata.tag);
        if (iter != tag_filter_.end())
            return (metadata.severity >= iter->second);

        iter = tag_filter_.find("*");
        if (iter != tag_filter_.end())
            return (metadata.severity >= iter->second);

        return false;
    }

    void add_filter(const Tag& tag, Severity severity)
    {
        tag_filter_[tag] = severity;
    }

    void add_filter(Severity severity)
    {
        tag_filter_["*"] = severity;
    }

    void add_filter(const std::string& filter)
    {
        auto pos = filter.find(":");
        if (pos != std::string::npos)
            add_filter(filter.substr(0, pos), to_severity(filter.substr(pos + 1)));
        else
            add_filter(to_severity(filter));
    }

private:
    std::map<Tag, Severity> tag_filter_;
};

} // namespace aixlog
