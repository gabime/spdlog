#include "spdlog/details/null_mutex.h"
#include "spdlog/async.h"

#include "spdlog/sinks/base_sink-inl.h"
template class spdlog::sinks::base_sink<std::mutex>;
template class spdlog::sinks::base_sink<spdlog::details::null_mutex>;

#include "spdlog/sinks/basic_file_sink-inl.h"
template class spdlog::sinks::basic_file_sink<std::mutex>;
template class spdlog::sinks::basic_file_sink<spdlog::details::null_mutex>;

#include "spdlog/sinks/rotating_file_sink-inl.h"
template class spdlog::sinks::rotating_file_sink<std::mutex>;
template class spdlog::sinks::rotating_file_sink<spdlog::details::null_mutex>;