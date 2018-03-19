//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init()
{
    logging::add_file_log(keywords::file_name = "logs/boost-bench-mt_%N.log", /*< file name pattern >*/
        keywords::auto_flush = false, keywords::format = "[%TimeStamp%]: %Message%");

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
}

using namespace std;

int main(int argc, char *argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = atoi(argv[1]);

    int howmany = 1000000;

    init();
    logging::add_common_attributes();

    using namespace logging::trivial;

    src::severity_logger_mt<severity_level> lg;

    std::atomic<int> msg_counter{0};
    vector<thread> threads;

    auto start = clock::now();
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]() {
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany)
                    break;
                BOOST_LOG_SEV(lg, info) << "boost message #" << counter << ": This is some text for your pleasure";
            }
        }));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Threads: " << thread_count << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;


    return 0;
}
