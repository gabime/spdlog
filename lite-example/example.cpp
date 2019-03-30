#include "spdlite.h"

int main()
{
    auto l = spdlite::create_logger();
    l.set_level(spdlite::level::trace);

    l.trace_printf("Hello %s ", "GABI");    
    l.info_printf("Hello %d", 12346);
    l.warn_printf("Hello %f", 12346.5656);
    l.warn("Hello {}", "LITE :) ");

    auto l2 = l.clone("logger2");    
    l2.debug("HELLO");

	auto l3 = std::move(l);
    l3.warn("HELLO FROM L3");

}