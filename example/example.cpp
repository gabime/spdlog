// example.cpp : Simple logger example
//
#define FFLOG_ENABLE_TRACE

#include <iostream>
#include "spitlog/logger.h"
#include "spitlog/sinks/stdout_sinks.h"
#include "spitlog/sinks/file_sinks.h"
using namespace std;
using namespace spitlog;


details::fast_oss f(const std::string& what)
{
    details::fast_oss oss;
    oss << what;
    return oss;
}
int main_(int, char* [])
{

    auto foss = f("test2");
    foss.str();

    return 0;
}

