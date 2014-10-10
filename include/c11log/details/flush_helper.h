#pragma once
// Flush to file every X writes..
// If X is zero than never flush..

namespace c11log
{
namespace details
{

class file_flush_helper
{
public:
    explicit file_flush_helper(const std::size_t flush_every):
        _flush_every(flush_every),
        _flush_countdown(flush_every) {};

    file_flush_helper(const file_flush_helper&) = delete;

    void write(const std::string& msg, std::ofstream& ofs)
    {
        ofs.write(msg.data(), msg.size());
        if(--_flush_countdown == 0)
        {
            ofs.flush();
            _flush_countdown = _flush_every;
        }
    }

private:
    const std::size_t _flush_every;
    std::size_t _flush_countdown;
};
}
}

