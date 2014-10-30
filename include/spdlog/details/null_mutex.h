#pragma once

// null, no cost mutex

namespace spdlog {
namespace details {
struct null_mutex
{
    void lock() {}
    void unlock() {}
    bool try_lock()
    {
        return true;
    }
};
}
}
