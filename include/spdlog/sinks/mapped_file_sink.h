#include "spdlog/common.h"
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/synchronous_factory.h>

#include <mutex>
#include <string>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <cstring>
#include <cstddef>

namespace spdlog {
namespace sinks {

template<typename Mutex>
class mapped_file_sink final : public base_sink<Mutex>
{
private:
    static constexpr std::size_t mem_buffer_size = 2 * 1024 * 1024;

    struct mmap_deleter
    {
        void operator()(char *p) const
        {
            if (munmap(p, mem_buffer_size) != 0)
                throw_spdlog_ex("munmap failed", errno);
        }
    };

    std::string path;
    std::size_t file_offset = 0;
    std::size_t mem_offset = 0;
    std::unique_ptr<char[], mmap_deleter> buffer;
    details::file_helper file_helper_;

public:
    explicit mapped_file_sink(const filename_t &filename, bool truncate = false, const file_event_handlers &event_handlers = {})
        : file_helper_{event_handlers}

    {
        file_helper_.open(filename, truncate);
        map_buffer();
    }

    mapped_file_sink(const mapped_file_sink &) = delete;
    mapped_file_sink(mapped_file_sink &&) noexcept = default;
    mapped_file_sink &operator=(const mapped_file_sink &) = delete;
    mapped_file_sink &operator=(mapped_file_sink &&) noexcept = default;

    ~mapped_file_sink() override
    {
        ftruncate(file_helper_.fileno(), file_offset + mem_offset);
    }

protected:
    void map_buffer()
    {
        ftruncate(file_helper_.fileno(), file_offset + mem_buffer_size);
        void *addr = mmap(nullptr, mem_buffer_size, PROT_WRITE, MAP_SHARED, file_helper_.fileno(), file_offset);
        if (addr == MAP_FAILED)
            throw_spdlog_ex("mmap failed", errno);
        memset(addr, 0, mem_buffer_size);
        buffer.reset(static_cast<char *>(addr));
        mem_offset = 0;
    }

    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        size_t written = 0;
        while (written < formatted.size())
        {
            auto to_write = std::min(formatted.size() - written, mem_buffer_size - mem_offset);
            std::memcpy(buffer.get() + mem_offset, formatted.data() + written, to_write);
            written += to_write;
            mem_offset += to_write;
            if (mem_offset == mem_buffer_size)
            {
                file_offset += mem_buffer_size;
                map_buffer();
            }
        }
    }
    void flush_() override
    {
        if (msync(buffer.get(), mem_buffer_size, MS_SYNC) != 0)
            throw_spdlog_ex("msync failed", errno);
    }
};

using mapped_file_sink_mt = mapped_file_sink<std::mutex>;
using mapped_file_sink_st = mapped_file_sink<details::null_mutex>;

} // namespace sinks

} // namespace spdlog