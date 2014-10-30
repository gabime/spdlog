#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <cstring>


// Fast memory storage on the stack when possible or in std::vector
namespace spdlog
{
namespace details
{

template<unsigned short STACK_SIZE>
class stack_buf
{
public:
    static const unsigned short stack_size = STACK_SIZE;
    stack_buf() :_v(), _stack_size(0) {}
    ~stack_buf() = default;
    stack_buf(const stack_buf& other):stack_buf(other, delegate_copy_move {})
    {}

    stack_buf(stack_buf&& other):stack_buf(other, delegate_copy_move {})
    {
        other.clear();
    }
    template<class T1>
    stack_buf& operator=(T1&& other)
    {
        _stack_size = other._stack_size;
        if (other.vector_used())
            _v = std::forward<T1>(other)._v;
        else
            std::copy_n(other._stack_array.begin(), other._stack_size, _stack_array.begin());
        return *this;
    }

    void append(const char* buf, std::size_t buf_size)
    {
        //If we are aleady using _v, forget about the stack
        if (vector_used())
        {
            _v.insert(_v.end(), buf, buf + buf_size);
        }
        //Try use the stack
        else
        {
            if (_stack_size + buf_size <= STACK_SIZE)
            {
                std::memcpy(&_stack_array[_stack_size], buf, buf_size);
                _stack_size += buf_size;
            }
            //Not enough stack space. Copy all to _v
            else
            {
                _v.reserve(_stack_size + buf_size);
                _v.insert(_v.end(), _stack_array.begin(), _stack_array.begin() + _stack_size);
                _v.insert(_v.end(), buf, buf + buf_size);
            }
        }
    }


    void clear()
    {
        _stack_size = 0;
        _v.clear();
    }

    const char* data() const
    {
        if (vector_used())
            return _v.data();
        else
            return _stack_array.data();
    }

    std::size_t size() const
    {
        if (vector_used())
            return _v.size();
        else
            return _stack_size;
    }

private:
    struct delegate_copy_move {};
    template<class T1>
    stack_buf(T1&& other, delegate_copy_move)
    {
        _stack_size = other._stack_size;
        if (other.vector_used())
            _v = std::forward<T1>(other)._v;
        else
            std::copy_n(other._stack_array.begin(), other._stack_size, _stack_array.begin());
    }

    inline bool vector_used() const
    {
        return !(_v.empty());
    }

    std::vector<char> _v;
    std::array<char, STACK_SIZE> _stack_array;
    std::size_t _stack_size;
};

}
} //namespace spdlog { namespace details {
