#include "spdlite.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define UNUSED(x) (void)(x)

// example of creating lite logger with console and file sink
spdlog::lite::logger spdlog::create_lite(void *ctx)
{
    UNUSED(ctx);
    std::shared_ptr<spdlog::logger> logger_impl;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt> ();
    console_sink->set_level(spdlog::level::debug);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt > ("log.txt", true);
    file_sink ->set_level(spdlog::level::info);

    logger_impl = std::make_shared<spdlog::logger>("my-logger", spdlog::sinks_init_list{console_sink, file_sink});
    logger_impl->set_level(spdlog::level::debug);
    return spdlog::lite::logger(std::move(logger_impl));
}
