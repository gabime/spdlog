// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
    #include <spdlog/pattern_formatter.h>
#endif

#include <spdlog/details/log_msg.h>
#include <spdlog/details/os.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/formatter.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace spdlog {

SPDLOG_INLINE pattern_formatter::pattern_formatter(std::string pattern,
                                                   pattern_time_type time_type,
                                                   std::string eol,
                                                   custom_flags custom_user_flags)
    : pattern_(std::move(pattern)),
      eol_(std::move(eol)),
      pattern_time_type_(time_type),
      need_localtime_(false),
      last_log_secs_(0),
      custom_handlers_(std::move(custom_user_flags)) {
    std::memset(&cached_tm_, 0, sizeof(cached_tm_));
    compile_pattern_(pattern_);
}

// use by default full formatter for if pattern is not given
SPDLOG_INLINE pattern_formatter::pattern_formatter(pattern_time_type time_type, std::string eol)
    : pattern_("%+"),
      eol_(std::move(eol)),
      pattern_time_type_(time_type),
      need_localtime_(true),
      last_log_secs_(0) {
    std::memset(&cached_tm_, 0, sizeof(cached_tm_));
    formatters_.push_back(details::make_unique<details::full_formatter>(details::padding_info{}));
}

SPDLOG_INLINE std::unique_ptr<formatter> pattern_formatter::clone() const {
    custom_flags cloned_custom_formatters;
    for (auto &it : custom_handlers_) {
        cloned_custom_formatters[it.first] = it.second->clone();
    }
    auto cloned = details::make_unique<pattern_formatter>(pattern_, pattern_time_type_, eol_,
                                                          std::move(cloned_custom_formatters));
    cloned->need_localtime(need_localtime_);
#if defined(__GNUC__) && __GNUC__ < 5
    return std::move(cloned);
#else
    return cloned;
#endif
}

SPDLOG_INLINE void pattern_formatter::format(const details::log_msg &msg, memory_buf_t &dest) {
    if (need_localtime_) {
        const auto secs =
            std::chrono::duration_cast<std::chrono::seconds>(msg.time.time_since_epoch());
        if (secs != last_log_secs_) {
            cached_tm_ = get_time_(msg);
            last_log_secs_ = secs;
        }
    }

    for (auto &f : formatters_) {
        f->format(msg, cached_tm_, dest);
    }
    // write eol
    details::fmt_helper::append_string_view(eol_, dest);
}

SPDLOG_INLINE void pattern_formatter::set_pattern(std::string pattern) {
    pattern_ = std::move(pattern);
    need_localtime_ = false;
    compile_pattern_(pattern_);
}

SPDLOG_INLINE void pattern_formatter::need_localtime(bool need) { need_localtime_ = need; }

SPDLOG_INLINE std::tm pattern_formatter::get_time_(const details::log_msg &msg) {
    if (pattern_time_type_ == pattern_time_type::local) {
        return details::os::localtime(log_clock::to_time_t(msg.time));
    }
    return details::os::gmtime(log_clock::to_time_t(msg.time));
}

template <typename Padder>
SPDLOG_INLINE void pattern_formatter::handle_flag_(char flag, details::padding_info padding) {
    // process custom flags
    auto it = custom_handlers_.find(flag);
    if (it != custom_handlers_.end()) {
        auto custom_handler = it->second->clone();
        custom_handler->set_padding_info(padding);
        formatters_.push_back(std::move(custom_handler));
        return;
    }

    // process built-in flags
    switch (flag) {
        case ('+'):  // default formatter
            formatters_.push_back(details::make_unique<details::full_formatter>(padding));
            need_localtime_ = true;
            break;

        case 'n':  // logger name
            formatters_.push_back(details::make_unique<details::name_formatter<Padder>>(padding));
            break;

        case 'l':  // level
            formatters_.push_back(details::make_unique<details::level_formatter<Padder>>(padding));
            break;

        case 'L':  // short level
            formatters_.push_back(
                details::make_unique<details::short_level_formatter<Padder>>(padding));
            break;

        case ('t'):  // thread id
            formatters_.push_back(details::make_unique<details::t_formatter<Padder>>(padding));
            break;

        case ('v'):  // the message text
            formatters_.push_back(details::make_unique<details::v_formatter<Padder>>(padding));
            break;

        case ('a'):  // weekday
            formatters_.push_back(details::make_unique<details::a_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('A'):  // short weekday
            formatters_.push_back(details::make_unique<details::A_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('b'):
        case ('h'):  // month
            formatters_.push_back(details::make_unique<details::b_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('B'):  // short month
            formatters_.push_back(details::make_unique<details::B_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('c'):  // datetime
            formatters_.push_back(details::make_unique<details::c_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('C'):  // year 2 digits
            formatters_.push_back(details::make_unique<details::C_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('Y'):  // year 4 digits
            formatters_.push_back(details::make_unique<details::Y_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('D'):
        case ('x'):  // datetime MM/DD/YY
            formatters_.push_back(details::make_unique<details::D_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('m'):  // month 1-12
            formatters_.push_back(details::make_unique<details::m_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('d'):  // day of month 1-31
            formatters_.push_back(details::make_unique<details::d_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('H'):  // hours 24
            formatters_.push_back(details::make_unique<details::H_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('I'):  // hours 12
            formatters_.push_back(details::make_unique<details::I_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('M'):  // minutes
            formatters_.push_back(details::make_unique<details::M_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('S'):  // seconds
            formatters_.push_back(details::make_unique<details::S_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('e'):  // milliseconds
            formatters_.push_back(details::make_unique<details::e_formatter<Padder>>(padding));
            break;

        case ('f'):  // microseconds
            formatters_.push_back(details::make_unique<details::f_formatter<Padder>>(padding));
            break;

        case ('F'):  // nanoseconds
            formatters_.push_back(details::make_unique<details::F_formatter<Padder>>(padding));
            break;

        case ('E'):  // seconds since epoch
            formatters_.push_back(details::make_unique<details::E_formatter<Padder>>(padding));
            break;

        case ('p'):  // am/pm
            formatters_.push_back(details::make_unique<details::p_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('r'):  // 12 hour clock 02:55:02 pm
            formatters_.push_back(details::make_unique<details::r_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('R'):  // 24-hour HH:MM time
            formatters_.push_back(details::make_unique<details::R_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('T'):
        case ('X'):  // ISO 8601 time format (HH:MM:SS)
            formatters_.push_back(details::make_unique<details::T_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('z'):  // timezone
            formatters_.push_back(details::make_unique<details::z_formatter<Padder>>(padding));
            need_localtime_ = true;
            break;

        case ('P'):  // pid
            formatters_.push_back(details::make_unique<details::pid_formatter<Padder>>(padding));
            break;

        case ('^'):  // color range start
            formatters_.push_back(details::make_unique<details::color_start_formatter>(padding));
            break;

        case ('$'):  // color range end
            formatters_.push_back(details::make_unique<details::color_stop_formatter>(padding));
            break;

        case ('@'):  // source location (filename:filenumber)
            formatters_.push_back(
                details::make_unique<details::source_location_formatter<Padder>>(padding));
            break;

        case ('s'):  // short source filename - without directory name
            formatters_.push_back(
                details::make_unique<details::short_filename_formatter<Padder>>(padding));
            break;

        case ('g'):  // full source filename
            formatters_.push_back(
                details::make_unique<details::source_filename_formatter<Padder>>(padding));
            break;

        case ('#'):  // source line number
            formatters_.push_back(
                details::make_unique<details::source_linenum_formatter<Padder>>(padding));
            break;

        case ('!'):  // source funcname
            formatters_.push_back(
                details::make_unique<details::source_funcname_formatter<Padder>>(padding));
            break;

        case ('%'):  // % char
            formatters_.push_back(details::make_unique<details::ch_formatter>('%'));
            break;

        case ('u'):  // elapsed time since last log message in nanos
            formatters_.push_back(
                details::make_unique<details::elapsed_formatter<Padder, std::chrono::nanoseconds>>(
                    padding));
            break;

        case ('i'):  // elapsed time since last log message in micros
            formatters_.push_back(
                details::make_unique<details::elapsed_formatter<Padder, std::chrono::microseconds>>(
                    padding));
            break;

        case ('o'):  // elapsed time since last log message in millis
            formatters_.push_back(
                details::make_unique<details::elapsed_formatter<Padder, std::chrono::milliseconds>>(
                    padding));
            break;

        case ('O'):  // elapsed time since last log message in seconds
            formatters_.push_back(
                details::make_unique<details::elapsed_formatter<Padder, std::chrono::seconds>>(
                    padding));
            break;

#ifndef SPDLOG_NO_TLS  // mdc formatter requires TLS support
        case ('&'):
            formatters_.push_back(details::make_unique<details::mdc_formatter<Padder>>(padding));
            break;
#endif

        default:  // Unknown flag appears as is
            auto unknown_flag = details::make_unique<details::aggregate_formatter>();

            if (!padding.truncate_) {
                unknown_flag->add_ch('%');
                unknown_flag->add_ch(flag);
                formatters_.push_back((std::move(unknown_flag)));
            }
            // fix issue #1617 (prev char was '!' and should have been treated as funcname flag
            // instead of truncating flag) spdlog::set_pattern("[%10!] %v") => "[      main] some
            // message" spdlog::set_pattern("[%3!!] %v") => "[mai] some message"
            else {
                padding.truncate_ = false;
                formatters_.push_back(
                    details::make_unique<details::source_funcname_formatter<Padder>>(padding));
                unknown_flag->add_ch(flag);
                formatters_.push_back((std::move(unknown_flag)));
            }

            break;
    }
}

// Extract given pad spec (e.g. %8X, %=8X, %-8!X, %8!X, %=8!X, %-8!X, %+8!X)
// Advance the given it pass the end of the padding spec found (if any)
// Return padding.
SPDLOG_INLINE details::padding_info pattern_formatter::handle_padspec_(
    std::string::const_iterator &it, std::string::const_iterator end) {
    using details::padding_info;
    using details::scoped_padder;
    const size_t max_width = 64;
    if (it == end) {
        return padding_info{};
    }

    padding_info::pad_side side;
    switch (*it) {
        case '-':
            side = padding_info::pad_side::right;
            ++it;
            break;
        case '=':
            side = padding_info::pad_side::center;
            ++it;
            break;
        default:
            side = details::padding_info::pad_side::left;
            break;
    }

    if (it == end || !std::isdigit(static_cast<unsigned char>(*it))) {
        return padding_info{};  // no padding if no digit found here
    }

    auto width = static_cast<size_t>(*it) - '0';
    for (++it; it != end && std::isdigit(static_cast<unsigned char>(*it)); ++it) {
        auto digit = static_cast<size_t>(*it) - '0';
        width = width * 10 + digit;
    }

    // search for the optional truncate marker '!'
    bool truncate;
    if (it != end && *it == '!') {
        truncate = true;
        ++it;
    } else {
        truncate = false;
    }
    return details::padding_info{std::min<size_t>(width, max_width), side, truncate};
}

SPDLOG_INLINE void pattern_formatter::compile_pattern_(const std::string &pattern) {
    auto end = pattern.end();
    std::unique_ptr<details::aggregate_formatter> user_chars;
    formatters_.clear();
    for (auto it = pattern.begin(); it != end; ++it) {
        if (*it == '%') {
            if (user_chars)  // append user chars found so far
            {
                formatters_.push_back(std::move(user_chars));
            }

            auto padding = handle_padspec_(++it, end);

            if (it != end) {
                if (padding.enabled()) {
                    handle_flag_<details::scoped_padder>(*it, padding);
                } else {
                    handle_flag_<details::null_scoped_padder>(*it, padding);
                }
            } else {
                break;
            }
        } else  // chars not following the % sign should be displayed as is
        {
            if (!user_chars) {
                user_chars = details::make_unique<details::aggregate_formatter>();
            }
            user_chars->add_ch(*it);
        }
    }
    if (user_chars)  // append raw chars found so far
    {
        formatters_.push_back(std::move(user_chars));
    }
}
}  // namespace spdlog
