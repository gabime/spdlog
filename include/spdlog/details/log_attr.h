#pragma once

#include <string>
// #include <string_view>
#include <vector>
#include "attr_composer.h"
#include <spdlog/common.h>

namespace spdlog {
namespace details {

template<typename T>
struct is_string
    : public std::integral_constant<bool, std::is_convertible<T, std::string>::value || std::is_convertible<T, string_view_t>::value>
{};

template<typename T>
struct is_number : public std::integral_constant<bool, 
    std::is_integral<T>::value
    // || std::is_floating_point<T>::value
>
{};

struct attr
{
    std::string key;
    std::string value;

public:
    template<typename key_t, typename value_t, typename std::enable_if<is_string<key_t>::value, key_t>::type * = nullptr,
        typename std::enable_if<is_string<value_t>::value, value_t>::type * = nullptr>
    attr(key_t const &k, value_t const &v)
    {
        scramble(key, k);
        scramble(value, v);
    }

    template<typename key_t, typename value_t, typename std::enable_if<is_string<key_t>::value, key_t>::type * = nullptr,
        typename std::enable_if<is_number<value_t>::value, value_t>::type * = nullptr>
    attr(key_t const &k, value_t const &v)
    {
        scramble(key, k);
        value = std::to_string(v);
    }
};

} // namespace details

using attribute_list = std::vector<details::attr>;

} // namespace spdlog