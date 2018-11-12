//
// Copyright(c) 2018 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include "benchmark/benchmark.h"

#include "spdlog/spdlog.h"
#include "spdlog/details/pattern_formatter.h"

void bench_scoped_pad(benchmark::State &state, size_t wrapped_size, spdlog::details::padding_info padinfo)
{
    fmt::memory_buffer dest;
    for (auto _ : state)
    {
        {
            spdlog::details::scoped_pad p(wrapped_size, padinfo, dest);
            benchmark::DoNotOptimize(p);
            dest.clear();
        }
    }
}


void bench_formatter(benchmark::State &state, std::string pattern)
{
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>(pattern);
    fmt::memory_buffer dest;
    std::string logger_name = "logger-name";
    const char* text = "Hello. This is some message with length of 80                                   ";


    spdlog::details::log_msg msg(&logger_name, spdlog::level::info, text);
//    formatter->format(msg, dest);
//    printf("%s\n", fmt::to_string(dest).c_str());

    for (auto _ : state)
    {
        dest.clear();
        formatter->format(msg, dest);
        benchmark::DoNotOptimize(dest);
    }
}

void bench_formatters()
{
    // basic patterns(single flag)
    std::string all_flags = "+vtPnlLaAbBcCYDmdHIMSefFprRTXzEi%";
    std::vector<std::string> basic_patterns;
    for(auto &flag:all_flags)
    {
        auto pattern = std::string("%") + flag;
        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);

    }

    // complex patterns
    std::vector<std::string> patterns = {
            "[%D %X] [%l] [%n] %v",
            "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v",
            "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [%t] %v",
    };
    for(auto &pattern:patterns)
    {
        benchmark::RegisterBenchmark(pattern.c_str(), bench_formatter, pattern);
    }
}

void bench_padders()
{
    using spdlog::details::padding_info;
    std::vector<size_t> sizes = {0, 2, 4, 8, 16, 32, 64, 128};

    for (auto size : sizes)
    {
        size_t wrapped_size = 8;
        size_t padding_size = wrapped_size + size;

        std::string title = "scoped_pad::left::" + std::to_string(size);

        benchmark::RegisterBenchmark(title.c_str(), bench_scoped_pad, wrapped_size, padding_info(padding_size, padding_info::left));

        title = "scoped_pad::right::" + std::to_string(size);
        benchmark::RegisterBenchmark(title.c_str(), bench_scoped_pad, wrapped_size, padding_info(padding_size, padding_info::right));

        title = "scoped_pad::center::" + std::to_string(size);
        benchmark::RegisterBenchmark(title.c_str(), bench_scoped_pad, wrapped_size, padding_info(padding_size, padding_info::center));
    }
}



int main(int argc, char *argv[])
{
    bench_formatters();
    //bench_padders();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}

