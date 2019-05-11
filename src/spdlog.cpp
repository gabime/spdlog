#include "spdlog/spdlog.h"

#include "spdlog/details/null_mutex.h"

#include "spdlog/logger.h"
#include "spdlog/logger-inl.h"

#include "spdlog/async_logger.h"
#include "spdlog/async_logger-inl.h"

#include "spdlog/details/log_msg.h"
#include "spdlog/details/log_msg-inl.h"

#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/sink-inl.h"

#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/base_sink-inl.h"
template class spdlog::sinks::base_sink<std::mutex>;
template class spdlog::sinks::base_sink<spdlog::details::null_mutex>;

#include "spdlog/details/registry.h"
#include "spdlog/details/registry-inl.h"

#include "spdlog/details/os.h"
#include "spdlog/details/os-inl.h"

#include "spdlog/details/periodic_worker.h"
#include "spdlog/details/periodic_worker-inl.h"

#include "spdlog/details/file_helper.h"
#include "spdlog/details/file_helper-inl.h"

#include "spdlog/details/pattern_formatter.h"
#include "spdlog/details/pattern_formatter-inl.h"

#include "spdlog/details/thread_pool.h"
#include "spdlog/details/thread_pool-inl.h"
template class spdlog::details::mpmc_blocking_queue<spdlog::details::async_msg>;

#include "spdlog/sinks/ansicolor_sink.h"
#include "spdlog/sinks/ansicolor_sink-inl.h"
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stdout, spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stdout, spdlog::details::console_nullmutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stderr, spdlog::details::console_mutex>;
template class spdlog::sinks::ansicolor_sink<spdlog::details::console_stderr, spdlog::details::console_nullmutex>;
