#include "includes.h"
#include "spdlog/sinks/relay_sink.h"
#include "spdlog/logger.h"
#include "test_sink.h"

TEST_CASE("relay_sink_test1", "[relay_sink]") {
    using spdlog::sinks::relay_sink_st;
    using spdlog::sinks::test_sink_mt;

    auto test_sink = std::make_shared<test_sink_mt>();
    auto remote_logger = std::make_shared<spdlog::logger>("remote", test_sink);
    auto relay_sink = std::make_shared<relay_sink_st>(remote_logger);

    for (int i = 0; i < 10; i++) {
        relay_sink->log(spdlog::details::log_msg{"test", spdlog::level::info, "message1"});
    }

    REQUIRE(test_sink->msg_counter() == 10);
}
