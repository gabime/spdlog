//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <thread>
#include <vector>
#include <atomic>
#include <cstdlib>
#include "spdlog/spdlog.h"


using namespace std;

int main(int argc, char* argv[])
{

    int thread_count = 10;
    if(argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;

    namespace spd = spdlog;

    auto logger = spdlog::create<spd::sinks::simple_file_sink_mt>("file_logger", "logs/spd-bench-mt.txt", false);

    logger->set_pattern("[%Y-%b-%d %T.%e]: %v");

    std::atomic<int > msg_counter {0};
    std::vector<thread> threads;

    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]()
        {
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany) break;
                logger->info("spdlog message #{}: This is some text for your pleasure", counter);
            }
        }));
    }


    for(auto &t:threads)
    {
        t.join();
    };



    return 0;
}
