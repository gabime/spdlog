#pragma once

#include <array>
#include <vector>
#include <algorithm>


// Fast memory storage
// stores its contents on the stack when possible, in vector<char> otherwise
// NOTE: User should be remember that returned buffer might be on the stack!!

namespace c11log
{
namespace details
{

template<std::size_t STACK_SIZE=128>
class fast_buf
{
public:
    fast_buf():_stack_size(0) {}
    ~fast_buf() {};

    fast_buf(const bufpair_t& buf_to_copy):fast_buf()
    {
        append(buf_to_copy);
    }

    fast_buf(const fast_buf& other)
    {
        _stack_size = other._stack_size;
        if(!other._v.empty())
            _v = other._v;
        else if(_stack_size)
            std::copy(other._stack_buf.begin(), other._stack_buf.begin()+_stack_size, _stack_buf.begin());
    }

    fast_buf(fast_buf&& other)
    {
        _stack_size = other._stack_size;
        if(!other._v.empty())
            _v = other._v;
        else if(_stack_size)
            std::copy(other._stack_buf.begin(), other._stack_buf.begin()+_stack_size, _stack_buf.begin());
        other.clear();
    }

    fast_buf& operator=(const fast_buf& other) = delete;
    fast_buf& operator=(fast_buf&& other) = delete;

    void append(const char* buf, std::size_t size)
    {
        //If we are aleady using _v, forget about the stack
        if(!_v.empty())
        {
            _v.insert(_v.end(), buf, buf+ size);
        }
        //Try use the stack
        else
        {
            if(_stack_size+size <= STACK_SIZE)
            {
                std::memcpy(&_stack_buf[_stack_size], buf, size);
                _stack_size+=size;
            }
            //Not enough stack space. Copy all to _v
            else
            {
                _v.reserve(_stack_size+size);
                if(_stack_size)
                    _v.insert(_v.end(), _stack_buf.begin(), _stack_buf.begin() +_stack_size);
                _v.insert(_v.end(), buf, buf+size);
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

private:
    std::vector<char> _v;
    std::array<char, STACK_SIZE> _stack_buf;
    std::size_t _stack_size;
};

}
} //namespace c11log { namespace details {
