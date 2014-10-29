#pragma once

#include <iostream>
#include <mutex>
#include "./ostream_sink.h"
#include "../details/null_mutex.h"

namespace spitlog
{
namespace sinks
{

template <class Mutex>
class stdout_sink : public ostream_sink<Mutex>
{
public:
    stdout_sink() : ostream_sink<Mutex>(std::cout) {}
};

typedef stdout_sink<details::null_mutex> stdout_sink_st;
typedef stdout_sink<std::mutex> stdout_sink_mt;


template <class Mutex>
class stderr_sink : public ostream_sink<Mutex>
{
public:
    stderr_sink() : ostream_sink<Mutex>(std::cerr) {}
};

typedef stderr_sink<std::mutex> stderr_sink_mt;
typedef stderr_sink<details::null_mutex> stderr_sink_st;
}
}