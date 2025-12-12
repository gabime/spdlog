#include <spdlog/spdlog.h>

int main() {
    spdlog::info("fixture_null_deref start");

    int *p = nullptr;
    *p = 1;

    spdlog::info("fixture_null_deref end {}", *p);
    return 0;
}
