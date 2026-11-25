#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // Workaround for GCC 12
#endif

#include <catch2/catch_all.hpp>

#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic pop
#endif

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace {

// drop_all()/drop()/set_default_logger(nullptr) permanently destroy the
// registry's default logger, making the suite order-dependent (Catch2 >= 3.16
// runs test cases in random order by default). Restore a fresh one before
// each test case.
class DefaultLoggerGuard : public Catch::EventListenerBase {
public:
    using EventListenerBase::EventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const &) override {
#ifndef SPDLOG_DISABLE_DEFAULT_LOGGER
        spdlog::set_default_logger(std::make_shared<spdlog::logger>(
            "", std::make_shared<spdlog::sinks::stdout_color_sink_mt>()));
#endif  // SPDLOG_DISABLE_DEFAULT_LOGGER
    }
};

}  // namespace

CATCH_REGISTER_LISTENER(DefaultLoggerGuard)
