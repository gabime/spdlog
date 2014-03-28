#pragma once

#include <array>
#include <vector>
#include <algorithm>
#include <cstring>

// Fast memory storage
// stores its contents on the stack when possible, in vector<char> otherwise
// NOTE: User should be remember that returned buffer might be on the stack!!

namespace c11log
{
namespace details
{

template<std::size_t STACK_SIZE=128>
class stack_buf
{
public:
    stack_buf():_stack_size(0) {}
    ~stack_buf() {};

    stack_buf(const bufpair_t& buf_to_copy):stack_buf()
    {
        append(buf_to_copy);
    }

    stack_buf(const stack_buf& other)
    {
        _stack_size = other._stack_size;
        if(!other._v.empty())
            _v = other._v;
        else if(_stack_size)
            std::copy(other._stack_buf.begin(), other._stack_buf.begin()+_stack_size, _stack_buf.begin());
    }

    stack_buf(stack_buf&& other)
    {
        _stack_size = other._stack_size;
        if(!other._v.empty())
            _v = other._v;
        else if(_stack_size)
            std::copy(other._stack_buf.begin(), other._stack_buf.begin()+_stack_size, _stack_buf.begin());
        other.clear();
    }

    stack_buf& operator=(const stack_buf& other) = delete;
    stack_buf& operator=(stack_buf&& other) = delete;

    void append(const char* buf, std::size_t buf_size)
    {
        //If we are aleady using _v, forget about the stack
        if(!_v.empty())
        {
            _v.insert(_v.end(), buf, buf+ buf_size);
        }
        //Try use the stack
        else
        {
            if(_stack_size+buf_size <= STACK_SIZE)
            {
                std::memcpy(&_stack_buf[_stack_size], buf, buf_size);
                _stack_size+=buf_size;
            }
            //Not enough stack space. Copy all to _v
            else
            {
                _v.reserve(_stack_size+buf_size);
                if(_stack_size)
                    _v.insert(_v.end(), _stack_buf.begin(), _stack_buf.begin() +_stack_size);
                _v.insert(_v.end(), buf, buf+buf_size);
            }
        }
    }

    void append(const bufpair_t &buf)
    {
        append(buf.first, buf.second);
    }

    void clear()
    {
        _stack_size = 0;
        _v.clear();
    }

    bufpair_t get()
    {
        if(!_v.empty())
            return bufpair_t(_v.data(), _v.size());
        else
            return bufpair_t(_stack_buf.data(), _stack_size);
    }

    std::size_t size()
    {
        if(!_v.empty())
            return _v.size();
        else
            return _stack_size;
    }

private:
    std::vector<char> _v;
    std::array<char, STACK_SIZE> _stack_buf;
    std::size_t _stack_size;
};

}
} //namespace c11log { namespace details {
