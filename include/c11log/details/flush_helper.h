#pragma once

// Flush to file every X writes..

namespace c11log {
namespace details {
class file_flush_helper {
public:
    explicit file_flush_helper(std::size_t flush_every): _flush_every(flush_every), _write_counter(0) {};

    void write(std::ofstream& ofs, const std::string& msg) {
        ofs << msg;
        if(++_write_counter >= _flush_every) {
            ofs.flush();
            _write_counter = 0;
        }
    }

private:
    std::size_t _flush_every;
    std::size_t _write_counter;
};
}
}

