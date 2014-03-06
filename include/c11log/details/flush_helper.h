#pragma once
#include <chrono>
#include <iostream>
// Flush to file every X writes..

namespace c11log
{
namespace details
{
class file_flush_helper
{
public:
    explicit file_flush_helper(const std::chrono::milliseconds &flush_every): _flush_every(flush_every), _last_flush() {};

    void write(std::ofstream& ofs, const std::string& msg) {


        ofs << msg;
        //If zero - flush every time
        if(_flush_every == std::chrono::milliseconds::min()) {
            ofs.flush();
        } else {
            auto now = std::chrono::system_clock::now();
            if(now - _last_flush >= _flush_every) {
                ofs.flush();
                _last_flush = now;
            }
        }
    }

private:
    std::chrono::milliseconds _flush_every;
    std::chrono::system_clock::time_point _last_flush;
};
}
}

