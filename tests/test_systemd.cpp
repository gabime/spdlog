#include "includes.h"
#include <spdlog/sinks/systemd_sink.h>

TEST_CASE("systemd", "[all]")
{
    auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
    systemd_sink->set_level(spdlog::level::level_enum::err);
    spdlog::logger logger("spdlog_systemd_test", systemd_sink);

    logger.debug("test debug");
    logger.error("test error");
    logger.info("test info");
}
