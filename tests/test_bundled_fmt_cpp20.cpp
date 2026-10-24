// Regression guard for https://github.com/gabime/spdlog/issues/3500
// (constexpr format parsing with bundled fmt on C++20).
#include <spdlog/spdlog.h>

void spdlog_bundled_fmt_cpp20_compile_check() {
    spdlog::default_logger()->info("{}", 3500);
}
