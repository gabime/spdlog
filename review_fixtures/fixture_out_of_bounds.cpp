#include <array>
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("fixture_out_of_bounds start");

    std::array<int, 4> a{{1, 2, 3, 4}};
    int x = a[10];

    spdlog::info("fixture_out_of_bounds end {}", x);
    return 0;
}
