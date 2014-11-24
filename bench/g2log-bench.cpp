#include "g2logworker.h"
#include "g2log.h"


int main(int, char* argv[])
{
    int howmany = 1000000;
	
	g2LogWorker g2log(argv[0], "logs");
    g2::initializeLogging(&g2log);
      
    for(int i  = 0 ; i < howmany; ++i)
        LOG(INFO) << "g2log message # " << i << ": This is some text for your pleasure";

    return 0;
}
