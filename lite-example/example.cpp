#include "spdlite.h"

int main()
{   
	spdlite::default_logger().set_level(spdlite::level::trace);
    spdlite::trace_printf("Hello %d", 123);    
	spdlite::debug_printf("Hello %d", 123);
    spdlite::info_printf("Hello %d", 123);
    spdlite::warn_printf("Hello %d", 123);
    spdlite::error_printf("Hello %d", 123);
    spdlite::critical_printf("Hello %d", 123);
}