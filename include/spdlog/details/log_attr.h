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
    // string overloads
    attr(string_view_t k, string_view_t v)
    {
        scramble(key, k);
        scramble(value, v);
    }

    // does not convert to string_view when using initializer list constructors...
    // so we have an overload for raw c-strings
    attr(const char* k, const char* v)
        : attr{string_view_t{k}, string_view_t{v}}
    {}

    attr(std::string const& k, std::string const& v)
        : attr{string_view_t{k}, string_view_t{v}}
    {}
    
    attr(std::string&& k, std::string&& v)
        : attr{string_view_t{k}, string_view_t{v}}
    {}

    // integer overloads
    attr(string_view_t k, long v)
        : value{std::to_string(v)}
    {
        scramble(key, k);
    }
    attr(string_view_t k, long long v)
        : value{std::to_string(v)}
    {
        scramble(key, k);
    }
    attr(string_view_t k, unsigned long v)
        : value{std::to_string(v)}
    {
        scramble(key, k);
    }
    attr(string_view_t k, unsigned long long v)
        : value{std::to_string(v)}
    {
        scramble(key, k);
    }
    
    attr(string_view_t k, bool v)
        : value{v ? "true" : "false"}
    {
        scramble(key, k);
    }
    
    attr(std::string const& k, long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string const& k, long long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string const& k, unsigned long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string const& k, unsigned long long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string const& k, bool v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string&& k, long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string&& k, long long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string&& k, unsigned long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string&& k, unsigned long long v)
        : attr{string_view_t{k}, v}
    {}

    attr(std::string&& k, bool v)
        : attr{string_view_t{k}, v}
    {}
};

} // namespace details

using attribute_list = std::vector<details::attr>;

} // namespace spdlog