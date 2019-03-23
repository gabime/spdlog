#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

int main()
{
    using namespace spdlog;
    auto l = spdlog::default_logger();
    l->set_level(spdlog::level::trace);

    l->trace("hello123123213 ");

}