// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/os.h"
#include "spdlog/formatter.h"

#include <chrono>
#include <ctime>
#include <memory>

#include <string>
#include <vector>

namespace spdlog {
namespace details {

// padding information.
struct padding_info
{
    enum pad_side
    {
        left,
        right,
        center
    };

    padding_info() = default;
    padding_info(size_t width, padding_info::pad_side side)
        : width_(width)
        , side_(side)
    {}

    bool enabled() const
    {
        return width_ != 0;
    }
    const size_t width_ = 0;
    const pad_side side_ = left;
};

class flag_formatter
{
public:
    explicit flag_formatter(padding_info padinfo)
        : padinfo_(padinfo)
    {}
    flag_formatter() = default;
    virtual ~flag_formatter() = default;
    virtual void format(const details::log_msg &msg, const std::tm &tm_time, fmt::memory_buffer &dest) = 0;

protected:
    padding_info padinfo_;
};

} // namespace details

class pattern_formatter final : public formatter
{
public:
    explicit pattern_formatter(
        std::string pattern, pattern_time_type time_type = pattern_time_type::local, std::string eol = spdlog::details::os::default_eol);

    // use default pattern is not given
    explicit pattern_formatter(pattern_time_type time_type = pattern_time_type::local, std::string eol = spdlog::details::os::default_eol);

    pattern_formatter(const pattern_formatter &other) = delete;
    pattern_formatter &operator=(const pattern_formatter &other) = delete;

    std::unique_ptr<formatter> clone() const override;
    void format(const details::log_msg &msg, fmt::memory_buffer &dest) override;

private:
    std::string pattern_;
    std::string eol_;
    pattern_time_type pattern_time_type_;
    std::tm cached_tm_;
    std::chrono::seconds last_log_secs_;
    std::vector<std::unique_ptr<details::flag_formatter>> formatters_;

    std::tm get_time_(const details::log_msg &msg);
    void handle_flag_(char flag, details::padding_info padding);

    // Extract given pad spec (e.g. %8X)
    // Advance the given it pass the end of the padding spec found (if any)
    // Return padding.
    details::padding_info handle_padspec_(std::string::const_iterator &it, std::string::const_iterator end);

    void compile_pattern_(const std::string &pattern);
};
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "pattern_formatter-inl.h"
#endif
