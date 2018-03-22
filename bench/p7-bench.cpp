//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#include <chrono>
#include <iostream>
#include <memory>
#include <functional>

#include "P7_Trace.h"


int main(int, char *[])
{
    using namespace std::chrono;
    using clock = steady_clock;

    int howmany = 1000000;

    IP7_Trace::hModule module = NULL;

    //create P7 client object
    std::unique_ptr<IP7_Client, std::function<void (IP7_Client *)>> client(
        P7_Create_Client(TM("/P7.Pool=1024 /P7.Sink=FileTxt /P7.Dir=logs/p7-bench")),
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

    auto start = clock::now();
    for (int i = 0; i < howmany; ++i)
        trace->P7_INFO(module, TM("p7 message #%d: This is some text for your pleasure"), i);


    duration<float> delta = clock::now() - start;
    float deltaf = delta.count();
    auto rate = howmany / deltaf;

    std::cout << "Total: " << howmany << std::endl;
    std::cout << "Delta = " << deltaf << " seconds" << std::endl;
    std::cout << "Rate = " << rate << "/sec" << std::endl;

    trace->Unregister_Thread(0);

    return 0;
}
