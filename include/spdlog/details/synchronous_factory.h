// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

namespace spdlog {

// Default logger factory-  creates synchronous loggers
class logger;

struct synchronous_factory
{
    template<typename Sink, typename... SinkArgs>
    static std::shared_ptr<spdlog::logger> create(std::string logger_name, SinkArgs &&... args)
    {
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        return std::make_shared<spdlog::logger>(std::move(logger_name), std::move(sink));
    }
};
} // namespace spdlog
