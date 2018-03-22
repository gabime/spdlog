//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <functional>

#include "P7_Trace.h"


int main(int argc, char *argv[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int thread_count = 10;
    if (argc > 1)
        thread_count = std::atoi(argv[1]);

    int howmany = 1000000;

    IP7_Trace::hModule module = NULL;

    //create P7 client object
    std::unique_ptr<IP7_Client, std::function<void (IP7_Client *)>> client(
        P7_Create_Client(TM("/P7.Pool=1024 /P7.Sink=FileTxt /P7.Dir=logs/p7-bench-mt")),
        [&](IP7_Client *ptr){
            if (ptr)
                ptr->Release();
        });

    if (!client)
    {
        std::cout << "Can't create IP7_Client" << std::endl;
        return 1;
    }

    //create P7 trace object 1
    std::unique_ptr<IP7_Trace, std::function<void (IP7_Trace *)>> trace(
        P7_Create_Trace(client.get(), TM("Trace channel 1")),
        [&](IP7_Trace *ptr){
            if (ptr)
                ptr->Release();
        });

    if (!trace)
    {
        std::cout << "Can't create IP7_Trace" << std::endl;
        return 1;
    }

    trace->Register_Thread(TM("Application"), 0);
    trace->Register_Module(TM("Main"), &module);

    std::atomic<int> msg_counter{0};
    std::vector<std::thread> threads;

    auto start = clock::now();
    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]() {
            trace->Register_Thread(TM("Application"), t+1);
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany)
                    break;
                trace->P7_INFO(module, TM("p7 message #%d: This is some text for your pleasure"), counter);
            }
            trace->Register_Thread(TM("Application"), t+1);
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

    trace->Unregister_Thread(0);

    return 0;
}
