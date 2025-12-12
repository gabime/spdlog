#include <spdlog/spdlog.h>
#include <thread>

static int g_counter = 0;

static void increment() {
    for (int i = 0; i < 100000; ++i) {
        g_counter++;
    }
}

int main() {
    spdlog::info("fixture_data_race start");

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    spdlog::info("fixture_data_race end {}", g_counter);
    return 0;
}
