//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//

#include "spdlog/logger.h"



spdlog::logger *get_logger();

int main(int, char *[])
{    
    auto *l = get_logger();
    l->info("HELLO  {}", "World");    
	l->warn("SOME WARNINNG");
    l->error("Some {}", "error");
   

}