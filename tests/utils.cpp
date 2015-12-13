#include "includes.h"

void prepare_logdir()
{
    spdlog::drop_all();
#ifdef _WIN32
    auto rv = system("del /F /Q logs\\*");
#else
    auto rv = system("rm -f logs/*");
#endif
}
