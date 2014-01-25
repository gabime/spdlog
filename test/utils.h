#pragma once

#include <functional>
#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <locale>

namespace utils
{

template<typename T>
std::string format(const T& value)
{
    static std::locale loc("");
    std::stringstream ss;
    ss.imbue(loc);
    ss << value;
    return ss.str();
}

inline void run(const std::chrono::milliseconds &duration, const std::function<void() >& fn)
{
    using namespace std::chrono;
    typedef steady_clock the_clock;
    size_t counter = 0;
    seconds print_interval(1);
    auto start_time = the_clock::now();
    auto lastPrintTime = start_time;
    while (true)
    {
        fn();
        ++counter;
        auto now = the_clock::now();
        if (now - start_time >= duration)
            break;
        auto p = now - lastPrintTime;
        if (now - lastPrintTime >= print_interval)
        {
            std::cout << format(counter) << " per sec" << std::endl;
            counter = 0;
            lastPrintTime = the_clock::now();
        }
    }
}
}