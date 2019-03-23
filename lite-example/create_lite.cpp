#include "spdlite.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"



spdlog::lite::logger spdlog::create_lite(void *ctx)
{
    if (ctx)
    {
        //..
    }
    auto logger_impl = spdlog::stdout_color_mt("mylogger");
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("file.txt", true);
    logger_impl->sinks().push_back(file_sink);
    logger_impl->set_level(spdlog::level::debug);
    return spdlog::lite::logger(std::move(logger_impl));
}
