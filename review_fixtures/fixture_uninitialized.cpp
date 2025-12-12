#include <spdlog/spdlog.h>

static int get_uninitialized_value(bool flag) {
    int x;
    if (flag) {
        x = 7;
    }
    return x;
}

int main() {
    spdlog::info("fixture_uninitialized start");

    int v = get_uninitialized_value(false);
    spdlog::info("fixture_uninitialized end {}", v);
    return 0;
}
