//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//


#include "easylogging++.h"

_INITIALIZE_EASYLOGGINGPP

int main(int, char* [])
{
    int howmany = 1000000;

    // Load configuration from file
    el::Configurations conf("easyl.conf");
    el::Loggers::reconfigureLogger("default", conf);

    for(int i  = 0 ; i < howmany; ++i)
        LOG(INFO) << "easylog message #" << i << ": This is some text for your pleasure";
    return 0;
}
