#pragma once
// Flush to file every X writes..

namespace c11log
{
namespace details
{

class file_flush_helper
{
public:
    explicit file_flush_helper(const std::size_t flush_every):
        _flush_every(flush_every),
        _write_counter(0) {};

    void write(const bufpair_t& msg, std::ofstream& ofs)
    {
        ofs.write(msg.first, msg.second);
        if(++_write_counter == _flush_every)
        {
            ofs.flush();
            _write_counter = 0;
        }
    }

private:
    const std::size_t _flush_every;
    std::size_t _write_counter;
};
}
}

