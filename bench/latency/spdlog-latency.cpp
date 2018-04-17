
#include "utils.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

namespace spd = spdlog;

namespace {
const uint64_t g_iterations = 1000000;

std::atomic<size_t> g_counter = {0};

void MeasurePeakDuringLogWrites(const size_t id, std::vector<uint64_t> &result)
{
    auto logger = spd::get("file_logger");
    while (true)
    {
        const size_t value_now = ++g_counter;
        if (value_now > g_iterations)
        {
            return;
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        logger->info("Some text to log for thread: [somemore text...............................] {}", id);
        auto stop_time = std::chrono::high_resolution_clock::now();
        uint64_t time_us = std::chrono::duration_cast<std::chrono::microseconds>(stop_time - start_time).count();
        result.push_back(time_us);
    }
}

void PrintResults(const std::map<size_t, std::vector<uint64_t>> &threads_result, size_t total_us)
{

    std::vector<uint64_t> all_measurements;
    all_measurements.reserve(g_iterations);
    for (auto &t_result : threads_result)
    {
        all_measurements.insert(all_measurements.end(), t_result.second.begin(), t_result.second.end());
    }

    // calc worst latenct
    auto worst = *std::max_element(all_measurements.begin(), all_measurements.end());

    // calc avg
    auto total = accumulate(begin(all_measurements), end(all_measurements), 0, std::plus<uint64_t>());
    auto avg = double(total) / all_measurements.size();

    std::cout << "[spdlog] worst: " << std::setw(10) << std::right << worst << "\tAvg: " << avg << "\tTotal: " << utils::format(total_us)
              << " us" << std::endl;
}
} // namespace

// The purpose of this test is NOT to see how fast
// each thread can possibly write. It is to see what
// the worst latency is for writing a log entry
//
// In the test 1 million log entries will be written
// an atomic counter is used to give each thread what
// it is to write next. The overhead of atomic
// synchronization between the threads are not counted in the worst case latency
int main(int argc, char **argv)
{
    size_t number_of_threads{0};
    if (argc == 2)
    {
        number_of_threads = atoi(argv[1]);
    }
    if (argc != 2 || number_of_threads == 0)
    {
        std::cerr << "usage: " << argv[0] << " number_threads" << std::endl;
        return 1;
    }

    std::vector<std::thread> threads(number_of_threads);
    std::map<size_t, std::vector<uint64_t>> threads_result;

    for (size_t idx = 0; idx < number_of_threads; ++idx)
    {
        // reserve to 1 million for all the result
        // it's a test so  let's not care about the wasted space
        threads_result[idx].reserve(g_iterations);
    }

    int queue_size = 1048576; // 2 ^ 20
    spdlog::set_async_mode(queue_size);
    auto logger = spdlog::create<spd::sinks::simple_file_sink_mt>("file_logger", "spdlog.log", true);

    // force flush on every call to compare with g3log
    auto s = (spd::sinks::simple_file_sink_mt *)logger->sinks()[0].get();
    s->set_force_flush(true);

    auto start_time_application_total = std::chrono::high_resolution_clock::now();
    for (uint64_t idx = 0; idx < number_of_threads; ++idx)
    {
        threads[idx] = std::thread(MeasurePeakDuringLogWrites, idx, std::ref(threads_result[idx]));
    }
    for (size_t idx = 0; idx < number_of_threads; ++idx)
    {
        threads[idx].join();
    }
    auto stop_time_application_total = std::chrono::high_resolution_clock::now();

    uint64_t total_time_in_us =
        std::chrono::duration_cast<std::chrono::microseconds>(stop_time_application_total - start_time_application_total).count();

    PrintResults(threads_result, total_time_in_us);
    return 0;
}
