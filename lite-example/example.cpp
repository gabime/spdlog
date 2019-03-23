#include "spdlite.h"

int main()
{
    auto l = spdlog::create_lite();
    l.set_level(spdlog::lite::level::trace);

    l.trace_f("Hello %s ","GABI");
    l.info_f("Hello %d", 12346);
    l.warn_f("Hello %f", 12346.5656);
    l.warn("Hello {}", 12346.5656);
}