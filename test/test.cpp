// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



int main(int argc, char* argv[])
{

    c11log::logger logger("test");

    auto screen_sink = std::make_shared<c11log::sinks::stdout_sink>();
    auto file_sink = std::make_shared<c11log::sinks::midnight_file_sink>("logtest");
    auto async = std::make_shared<c11log::sinks::async_sink>(1000);
    async->add_sink(file_sink);
    logger.add_sink(async);
    //logger.add_sink(file_sink);


    auto fn = [&logger]()
    {
        logger.info() << "Hello logger!";
    };
    utils::bench("test log", std::chrono::seconds(3), fn);
    logger.info() << "bye";
    utils::bench("shutdown", [&async]() {
        async->shutdown(std::chrono::seconds(10));
    });


    return 0;
}


