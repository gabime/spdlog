#include <mutex>
#include <spdlog/spdlog.h>
#include <thread>

static std::mutex m1;
static std::mutex m2;

static void lock_order_1() {
    std::lock_guard<std::mutex> l1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l2(m2);
}

static void lock_order_2() {
    std::lock_guard<std::mutex> l2(m2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> l1(m1);
}

int main() {
    spdlog::info("fixture_deadlock start");

    std::thread t1(lock_order_1);
    std::thread t2(lock_order_2);
    t1.join();
    t2.join();

    spdlog::info("fixture_deadlock end");
    return 0;
}
