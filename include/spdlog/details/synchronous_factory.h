// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "registry.h"

namespace spdlog {

// Default logger factory-  creates synchronous loggers
class logger;

struct synchronous_factory
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<spdlog::logger> create(std::string logger_name, SinkArgs &&... args)
    {
        std::shared_ptr<spdlog::logger> NewLogger;
        try
        {
            auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        }
        catch(std::bad_alloc& exc)
        {
            return NewLogger;
        }
        
        try
        {
            auto new_logger = std::make_shared<spdlog::logger>(std::move(logger_name), std::move(sink));
        }
        catch(std::bad_alloc& exc)
        {
            return NewLogger;
        }
        
        details::registry::instance().initialize_logger(new_logger);
        return NewLogger;
    }
};
} // namespace spdlog
