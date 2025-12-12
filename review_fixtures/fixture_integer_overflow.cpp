#include <climits>
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("fixture_integer_overflow start");

    int x = INT_MAX;
    int y = x + 1;

    spdlog::info("fixture_integer_overflow end {}", y);
    return 0;
}
