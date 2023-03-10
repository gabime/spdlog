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
    Key(std::string const& k) {
        scramble(_key, k);
    }
    Key(const char* k) {
        scramble(_key, k);
    }
};

struct Value
{
    std::string _value;

    // string types
    Value(string_view_t v) {
        scramble(_value, v);
    }
    Value(std::string&& v) {
        scramble(_value, v);
    }
    Value(std::string const& v) {
        scramble(_value, v);
    }
    Value(const char* v) {
        scramble(_value, v);
    }
    
    // integer types
    // probably better to do this with templates, but constraints are needed
    // concepts would be nice here, but spdlog is c++11
    // SFINAE is also an option, but it's a bit more complicated
    // https://stackoverflow.com/questions/41552514/is-overloading-on-all-of-the-fundamental-integer-types-is-sufficient-to-capture
    // basing the types off of MSVC, GCC, and Clang (https://en.cppreference.com/w/cpp/language/types)
    
    // chars are already strings (single character)
    Value(signed char v) {
        _value = v; 
    }
    Value(unsigned char v) {
        _value = v;
    }
    
    // these are overloads, which match the overloads in to_string for msvc, gcc, and clang
    Value(int v) {
        _value = std::to_string(v);
    }
    Value(unsigned int v) {
        _value = std::to_string(v);
    }
    Value(long v) {
        _value = std::to_string(v);
    }
    Value(unsigned long v) {
        _value = std::to_string(v);
    }
    Value(long long v) {
        _value = std::to_string(v);
    }
    Value(unsigned long long v) {
        _value = std::to_string(v);
    }
    Value(float v) {
        _value = std::to_string(v);
    }
    Value(double v) {
        _value = std::to_string(v);
    }
    Value(long double v) {
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