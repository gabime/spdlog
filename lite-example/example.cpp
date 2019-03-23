#include "spdlite.h"

int main()
{
    using namespace spdlog;
    auto l = spdlog::create_lite();
    l.set_level(spdlog::lite::level::trace);


    l.trace_f("Hello %s ","GABI");
    l.info_f("Hello %d", 12346);

}