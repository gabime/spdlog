#include <new>
#include <spdlog/spdlog.h>

int main() {
    spdlog::info("fixture_leak start");
    int *p = new int[128];
    p[0] = 42;

    spdlog::info("fixture_leak end {}", p[0]);
    return 0;
}
