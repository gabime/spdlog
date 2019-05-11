// Copyright(c) 2015-present Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma  once


SPDLOG_INLINE spdlog::spdlog_ex::spdlog_ex(std::string msg)
: msg_(std::move(msg))
{
}

SPDLOG_INLINE spdlog::spdlog_ex::spdlog_ex(const std::string &msg, int last_errno)
{
    fmt::memory_buffer outbuf;
    fmt::format_system_error(outbuf, last_errno, msg);
    msg_ = fmt::to_string(outbuf);
}

SPDLOG_INLINE const char *spdlog::spdlog_ex::what() const SPDLOG_NOEXCEPT
{
    return msg_.c_str();
}

