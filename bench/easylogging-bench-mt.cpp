#include <thread>
#include <vector>
#include <atomic>

#define _ELPP_THREAD_SAFE
#include "easylogging++.h"
_INITIALIZE_EASYLOGGINGPP

using namespace std;

int main(int argc, char* argv[])
{

    int thread_count = 10;
    if(argc > 1)
        thread_count = atoi(argv[1]);

    int howmany = 1000000;

    // Load configuration from file
    el::Configurations conf("easyl.conf");
    el::Loggers::reconfigureLogger("default", conf);

    std::atomic<int > msg_counter {0};
    vector<thread> threads;

    for (int t = 0; t < thread_count; ++t)
    {
        threads.push_back(std::thread([&]()
        {
            while (true)
            {
                int counter = ++msg_counter;
                if (counter > howmany) break;
                LOG(INFO) << "easylog message #" << counter << ": This is some text for your pleasure";
            }
        }));
    }


    for(auto &t:threads)
    {
        t.join();
    };

    return 0;
}
