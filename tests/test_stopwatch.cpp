#include "includes.h"
#include "test_sink.h"
#include "spdlog/stopwatch.h"

TEST_CASE("stopwatch1", "[stopwatch]")
{
    using std::chrono::milliseconds;
    using clock = std::chrono::steady_clock;
    milliseconds wait_ms(200);
    milliseconds tolerance_ms(250);
    auto start = clock::now();
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(wait_ms);
    auto stop = clock::now();
    auto diff_ms = std::chrono::duration_cast<milliseconds>(stop - start);
    REQUIRE(sw.elapsed() >= diff_ms);
    REQUIRE(sw.elapsed() <= diff_ms + tolerance_ms);
}

TEST_CASE("stopwatch2", "[stopwatch]")
{
    using spdlog::sinks::test_sink_st;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using clock = std::chrono::steady_clock;

    clock::duration wait_duration(milliseconds(200));
    clock::duration tolerance_duration(milliseconds(250));

    auto test_sink = std::make_shared<test_sink_st>();

    auto start = clock::now();
    spdlog::stopwatch sw;
    spdlog::logger logger("test-stopwatch", test_sink);
    logger.set_pattern("%v");
    std::this_thread::sleep_for(wait_duration);
    auto stop = clock::now();
    logger.info("{}", sw);
    auto val = std::stod(test_sink->lines()[0]);
    auto diff_duration = duration_cast<std::chrono::duration<double>>(stop - start);

    REQUIRE(val >= (diff_duration).count() - 0.001);
    REQUIRE(val <= (diff_duration + tolerance_duration).count());
}
