#include "spdlite.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/stdout_sinks.h"

#define UNUSED(x) (void)(x)

// example of creating lite logger with console and file sink
spdlite::logger create_logger(void *ctx)
{
    UNUSED(ctx);
    return spdlite::default_logger();    
}
