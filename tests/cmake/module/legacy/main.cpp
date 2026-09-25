// A deliberately old-style consumer: CMake 3.10, C++11, plain #include.
// It must keep building against an install that has the C++20 module enabled.
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>

int main() {
    auto logger = spdlog::null_logger_mt("legacy");
    logger->info("legacy consumer still works");
    SPDLOG_LOGGER_INFO(logger, "legacy macro still works");
    return 0;
}
