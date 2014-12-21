#include <thread>
#include <vector>
#include <atomic>
#include <iostream>
#include <chrono>

#include "g2logworker.h"
#include "g2log.h"

using namespace std;
template<typename T> std::string format(const T& value);

int main(int argc, char* argv[])
{
    using namespace std::chrono;
    using clock=steady_clock;
    int thread_count = 10;

    if(argc > 1)
        thread_count = atoi(argv[1]);
    int howmany = 1000000;

    g2LogWorker g2log(argv[0], "logs");
    g2::initializeLogging(&g2log);


    std::atomic<int > msg_counter {0};
    vector<thread> threads;
    auto start = clock::now();
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]()
        {
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany) break;
                LOG(INFO) << "g2log message #" << counter << ": This is some text for your pleasure";
            }
        }));
    }


    for(auto &t:threads)
    {
        t.join();
    };

    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany/deltaf;

    cout << "Total: " << howmany << std::endl;
    cout << "Threads: " << thread_count << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;
}
