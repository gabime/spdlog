#pragma once

#include <string>
#include <vector>
#include "attr_composer.h"
#include <spdlog/common.h>

namespace spdlog {
namespace details {

// template<typename T>
// concept composable = std::same_as<T, bool> || std::integral<T> || std::floating_point<T> || std::convertible_to<T, std::string_view>;

struct attr
{
    std::string key;
    std::string value;

public:
    attr(std::initializer_list<string_view_t> l)
    {
        if (l.size() != 2)
            return; // throw exception if not kv pair?

        scramble(key, *l.begin());
        scramble(value, *(l.begin() + 1));
    }

    attr(string_view_t k, bool v)
        : value{v ? "true" : "false"}
    {
        key = std::string{k.data(), k.size()};
    }

    attr(string_view_t k, string_view_t v)
    {
        key = std::string{k.data(), k.size()};
        value = std::string{v.data(), v.size()};
    }

    attr(std::string k, std::string v)
        : key{k}
        , value{v}
    {}

    template <typename T>
        requires std::integral<T> || std::floating_point<T>
    attr(string_view_t k, T v) : value{std::to_string(v)} {
        key = std::string{k.data(), k.size()};
    }

    attr(string_view_t k, long v)
        : value{std::to_string(v)}
    {
        key = std::string{k.data(), k.size()};
    }
    attr(string_view_t k, long long v)
        : value{std::to_string(v)}
    {
        key = std::string{k.data(), k.size()};
    }
    attr(string_view_t k, unsigned long v)
        : value{std::to_string(v)}
    {
        key = std::string{k.data(), k.size()};
    }
    attr(string_view_t k, unsigned long long v)
        : value{std::to_string(v)}
    {
        key = std::string{k.data(), k.size()};
    }
};

} // namespace details

using attribute_list = std::vector<details::attr>;

} // namespace spdlog