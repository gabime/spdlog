#include "includes.h"
#include <spdlog/sinks/systemd_sink.h>

namespace {
const char *tested_logger_name = "main";
}

void run(spdlog::logger &logger) {
    logger.debug("test debug");
    logger.error("test error");
    logger.info("test info");
}

// std::shared_ptr<spdlog::logger> create_stdout_and_systemd_logger(
//     std::string name="",
//     spdlog::level::level_enum level_stdout=spdlog::level::level_enum::debug,
//     spdlog::level::level_enum level_systemd=spdlog::level::level_enum::err
// ) {
//     auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//     console_sink->set_level(level_stdout);
//     auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
//     systemd_sink->set_level(level_systemd);
//     return std::make_shared<spdlog::logger>(name, {console_sink, systemd_sink});

// }

TEST_CASE("systemd", "[all]")
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::level_enum::debug);
    auto systemd_sink = std::make_shared<spdlog::sinks::systemd_sink_st>();
    systemd_sink->set_level(spdlog::level::level_enum::err);
    spdlog::logger logger("spdlog_systemd_test", {console_sink, systemd_sink});
    run(logger);
}
