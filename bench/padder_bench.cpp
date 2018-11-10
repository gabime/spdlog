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
        }
        //        if(dest.size() != (padinfo.width_-wrapped_size))
        //        {
        //            printf("NOT GOOD wrapped_size=%zu\t padinfo.width= %zu\tdest = %zu\n", wrapped_size, padinfo.width_, dest.size());
        //        }
        dest.clear();
    }
}

int main(int argc, char *argv[])
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

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
