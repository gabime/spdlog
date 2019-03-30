// Copyright(c) 2015-present Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlite.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"


#define UNUSED(x) (void)(x)

// example of creating lite logger with console and file sink
spdlite::logger create_logger(void *ctx)
{
    UNUSED(ctx);
    return spdlite::logger(spdlog::basic_logger_mt("logger-name", "log.txt", true));
}
