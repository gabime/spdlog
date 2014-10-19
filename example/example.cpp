// example.cpp : Simple logger example
//
#define FFLOG_ENABLE_TRACE

#include <iostream>
#include "c11log/logger.h"
#include "c11log/factory.h"
#include "c11log/sinks/stdout_sinks.h"
#include "c11log/sinks/file_sinks.h"
using namespace std;
using namespace c11log;


details::fast_oss f(const std::string& what)
{
    details::fast_oss oss;
    oss << what;
    return oss;
}
int main(int, char* [])
{

    auto foss = f("test2");
    foss.str();

    return 0;
}

