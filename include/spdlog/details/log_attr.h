#pragma once

#include <string>
#include <vector>
#include "attr_composer.h"
#include <spdlog/common.h>

namespace spdlog {
namespace details {

// template<typename T>
// concept composable = std::same_as<T, bool> || std::integral<T> || std::floating_point<T> || std::convertible_to<T, std::string_view>;


struct Key 
{
    std::string _key;

    Key(string_view_t k) {
        scramble(_key, k);
    }
    Key(std::string&& k) {
        scramble(_key, k);
    }
    Key(const char* k) {
        scramble(_key, k);
    }
};

struct Value
{
    std::string _value;

    Value(string_view_t v) {
        scramble(_value, v);
    }
    Value(std::string&& v) {
        scramble(_value, v);
    }
    Value(const char* v) {
        scramble(_value, v);
    }
    Value(long v) {
        _value = std::to_string(v);
    }
    Value(long long v) {
        _value = std::to_string(v);
    }
    Value(unsigned long v) {
        _value = std::to_string(v);
    }
    Value(unsigned long long v) {
        _value = std::to_string(v);
    }
    Value(bool v) {
        _value = v ? "true" : "false";
    }
};

struct attr
{
    std::string key;
    std::string value;

public:
    attr(Key&& k, Value&& v) : key(std::move(k._key)), value(std::move(v._value)) {}
    attr(Key const& k, Value const& v) : key(k._key), value(v._value) {}
};

} // namespace details

using attribute_list = std::vector<details::attr>;

} // namespace spdlog