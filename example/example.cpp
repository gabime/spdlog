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
    l->info("HE LO ", "GA");    	
	l->error("Some {} {} {}", "er or");
    l->error("Some {} {} {}", "er or");
   

}