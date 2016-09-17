//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <sstream>
#include <iomanip>
#include <locale>

namespace utils
{

template<typename T>
inline std::string format(const T& value)
{
    static std::locale loc("");
    std::stringstream ss;
    ss.imbue(loc);
    ss << value;
    return ss.str();
}

template<>
inline std::string format(const double & value)
{
    static std::locale loc("");
    std::stringstream ss;
    ss.imbue(loc);
    ss << std::fixed << std::setprecision(1) << value;
    return ss.str();
}

}
