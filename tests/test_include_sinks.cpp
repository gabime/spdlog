#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/callback_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/dup_filter_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/tcp_sink.h"
#include "spdlog/sinks/udp_sink.h"

#ifdef _WIN32
    #include "spdlog/sinks/msvc_sink.h"
    #include "spdlog/sinks/win_eventlog_sink.h"
#else
    #include "spdlog/sinks/syslog_sink.h"
#endif
