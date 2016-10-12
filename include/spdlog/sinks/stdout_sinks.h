//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <cstdio>
#include <memory>
#include <mutex>

namespace spdlog
{
namespace sinks
{

template <class Mutex>
class stdout_sink: public base_sink<Mutex>
{
    using MyType = stdout_sink<Mutex>;
public:
    stdout_sink()
    {}
    static std::shared_ptr<MyType> instance()
    {
        static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
        return instance;
    }

    void _sink_it(const details::log_msg& msg) override
    {
        fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), stdout);
        flush();
    }

    void flush() override
    {
        fflush(stdout);
    }
};

typedef stdout_sink<details::null_mutex> stdout_sink_st;
typedef stdout_sink<std::mutex> stdout_sink_mt;


template <class Mutex>
class stderr_sink: public base_sink<Mutex>
{
    using MyType = stderr_sink<Mutex>;
public:
    stderr_sink()
    {}
    static std::shared_ptr<MyType> instance()
    {
        static std::shared_ptr<MyType> instance = std::make_shared<MyType>();
        return instance;
    }

    void _sink_it(const details::log_msg& msg) override
    {
        fwrite(msg.formatted.data(), sizeof(char), msg.formatted.size(), stderr);
        flush();
    }

    void flush() override
    {
        fflush(stderr);
    }
};

typedef stderr_sink<std::mutex> stderr_sink_mt;
typedef stderr_sink<details::null_mutex> stderr_sink_st;
}
}
