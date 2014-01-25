// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


void fn();
int main(int argc, char* argv[])
{

    c11log::logger logger("test");

    auto sink = std::make_shared<c11log::sinks::stdout_sink>();
    auto async = std::make_shared<c11log::sinks::async_sink>(100);
    async->add_sink(sink);
    logger.add_sink(async);
    logger.info() << "Hello logger!";
    utils::run(std::chrono::seconds(10), fn);
    return 0;
}

static void fn()
{}

