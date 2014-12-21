
#include "glog/logging.h"


int main(int, char* argv[])
{
    int howmany = 1000000;


    FLAGS_logtostderr = 0;
    FLAGS_log_dir = "logs";
    google::InitGoogleLogging(argv[0]);
    for(int i  = 0 ; i < howmany; ++i)
        LOG(INFO) << "glog message # " << i << ": This is some text for your pleasure";

    return 0;
}
