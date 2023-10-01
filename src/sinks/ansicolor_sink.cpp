// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/sinks/ansicolor_sink.h"

#include <mutex>

#include "spdlog/details/null_mutex.h"
#include "spdlog/details/os.h"
#include "spdlog/pattern_formatter.h"

namespace {
// Formatting codes
constexpr spdlog::string_view_t reset = "\033[m";
constexpr spdlog::string_view_t bold = "\033[1m";
constexpr spdlog::string_view_t dark = "\033[2m";
constexpr spdlog::string_view_t underline = "\033[4m";
constexpr spdlog::string_view_t blink = "\033[5m";
constexpr spdlog::string_view_t reverse = "\033[7m";
constexpr spdlog::string_view_t concealed = "\033[8m";
constexpr spdlog::string_view_t clear_line = "\033[K";

// Foreground colors
constexpr spdlog::string_view_t black = "\033[30m";
constexpr spdlog::string_view_t red = "\033[31m";
constexpr spdlog::string_view_t green = "\033[32m";
constexpr spdlog::string_view_t yellow = "\033[33m";
constexpr spdlog::string_view_t blue = "\033[34m";
constexpr spdlog::string_view_t magenta = "\033[35m";
constexpr spdlog::string_view_t cyan = "\033[36m";
constexpr spdlog::string_view_t white = "\033[37m";

// Background colors
static constexpr spdlog::string_view_t on_black = "\033[40m";
constexpr spdlog::string_view_t on_red = "\033[41m";
constexpr spdlog::string_view_t on_green = "\033[42m";
constexpr spdlog::string_view_t on_yellow = "\033[43m";
constexpr spdlog::string_view_t on_blue = "\033[44m";
constexpr spdlog::string_view_t on_magenta = "\033[45m";
constexpr spdlog::string_view_t on_cyan = "\033[46m";
constexpr spdlog::string_view_t on_white = "\033[47m";

// Bold colors
constexpr spdlog::string_view_t yellow_bold = "\033[33m\033[1m";
constexpr spdlog::string_view_t red_bold = "\033[31m\033[1m";
constexpr spdlog::string_view_t bold_on_red = "\033[1m\033[41m";
}  // namespace

namespace spdlog {
namespace sinks {

template <typename Mutex>
ansicolor_sink<Mutex>::ansicolor_sink(FILE *target_file, color_mode mode)
    : target_file_(target_file)

{
    set_color_mode(mode);
    colors_.at(level_to_number(level::trace)) = to_string_(white);
    colors_.at(level_to_number(level::debug)) = to_string_(cyan);
    colors_.at(level_to_number(level::info)) = to_string_(green);
    colors_.at(level_to_number(level::warn)) = to_string_(yellow_bold);
    colors_.at(level_to_number(level::err)) = to_string_(red_bold);
    colors_.at(level_to_number(level::critical)) = to_string_(bold_on_red);
    colors_.at(level_to_number(level::off)) = to_string_(reset);
}

template <typename Mutex>
void ansicolor_sink<Mutex>::set_color(level color_level, string_view_t color) {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    colors_.at(level_to_number(color_level)) = to_string_(color);
}

template <typename Mutex>
bool ansicolor_sink<Mutex>::should_color() {
    return should_do_colors_;
}

template <typename Mutex>
void ansicolor_sink<Mutex>::set_color_mode(color_mode mode) {
    switch (mode) {
        case color_mode::always:
            should_do_colors_ = true;
            return;
        case color_mode::automatic:
            should_do_colors_ =
                details::os::in_terminal(target_file_) && details::os::is_color_terminal();
            return;
        case color_mode::never:
            should_do_colors_ = false;
            return;
        default:
            should_do_colors_ = false;
    }
}

template <typename Mutex>
void ansicolor_sink<Mutex>::sink_it_(const details::log_msg &msg) {
    // Wrap the originally formatted message in color codes.
    // If color is not supported in the terminal, log as is instead.

    msg.color_range_start = 0;
    msg.color_range_end = 0;
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    if (should_do_colors_ && msg.color_range_end > msg.color_range_start) {
        // before color range
        print_range_(formatted, 0, msg.color_range_start);
        // in color range
        print_ccode_(colors_.at(level_to_number(msg.log_level)));
        print_range_(formatted, msg.color_range_start, msg.color_range_end);
        print_ccode_(reset);
        // after color range
        print_range_(formatted, msg.color_range_end, formatted.size());
    } else  // no color
    {
        print_range_(formatted, 0, formatted.size());
    }
    fflush(target_file_);
}

template <typename Mutex>
void ansicolor_sink<Mutex>::flush_() {
    fflush(target_file_);
}

template <typename Mutex>
void ansicolor_sink<Mutex>::print_ccode_(const string_view_t &color_code) {
    fwrite(color_code.data(), sizeof(char), color_code.size(), target_file_);
}

template <typename Mutex>
void ansicolor_sink<Mutex>::print_range_(const memory_buf_t &formatted, size_t start, size_t end) {
    fwrite(formatted.data() + start, sizeof(char), end - start, target_file_);
}

template <typename Mutex>
std::string ansicolor_sink<Mutex>::to_string_(const string_view_t &sv) {
    return {sv.data(), sv.size()};
}

// ansicolor_stdout_sink
template <typename Mutex>
ansicolor_stdout_sink<Mutex>::ansicolor_stdout_sink(color_mode mode)
    : ansicolor_sink<Mutex>(stdout, mode) {}

// ansicolor_stderr_sink
template <typename Mutex>
ansicolor_stderr_sink<Mutex>::ansicolor_stderr_sink(color_mode mode)
    : ansicolor_sink<Mutex>(stderr, mode) {}

}  // namespace sinks
}  // namespace spdlog

// template instantiations
template SPDLOG_API class spdlog::sinks::ansicolor_stdout_sink<std::mutex>;
template SPDLOG_API class spdlog::sinks::ansicolor_stdout_sink<spdlog::details::null_mutex>;

template SPDLOG_API class spdlog::sinks::ansicolor_stderr_sink<std::mutex>;
template SPDLOG_API class spdlog::sinks::ansicolor_stderr_sink<spdlog::details::null_mutex>;
