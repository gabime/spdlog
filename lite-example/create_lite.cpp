#include "logger.h"
#include "spdlog/spdlog.h"


spdlog::lite::logger spdlog::create_lite(void* ctx)
{
    if(ctx) {
        //..
    }
    auto logger_impl = spdlog::default_logger();
    logger_impl->set_level(spdlog::level::trace);
    return spdlog::lite::logger(logger_impl);
}


