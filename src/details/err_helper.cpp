// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/details/err_helper.h"

#include "spdlog/details/os.h"

SPDLOG_NAMESPACE_BEGIN
namespace details {

err_helper::err_helper(const err_helper &other) {
    std::lock_guard lock(other.mutex_);
    custom_err_handler_ = other.custom_err_handler_;
    last_report_time_ = other.last_report_time_;
}

err_helper::err_helper(err_helper &&other) noexcept {
    custom_err_handler_ = std::move(other.custom_err_handler_);
    last_report_time_ = std::move(other.last_report_time_);
}

// Prints error to stderr with source location (if available). A stderr sink is not used because reaching
// this point might indicate a problem with the logging system itself so we use fputs() directly.
void err_helper::handle_ex(const std::string &origin, const source_loc &loc, const std::exception &ex) noexcept {
    std::lock_guard lock(mutex_);
    try {
        if (custom_err_handler_) {
            custom_err_handler_(ex.what());
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        if (now - last_report_time_ < std::chrono::seconds(1)) {
            return;
        }
        last_report_time_ = now;
        const auto tm_time = os::localtime();
        char date_buf[64];
        if (std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time) == 0) {
            std::snprintf(date_buf, sizeof(date_buf), "unknown time");
        }
        std::string msg;
        if (loc.empty()) {
            msg = fmt_lib::format("[*** LOG ERROR ***] [{}] [{}] {}\n", date_buf, origin, ex.what());
        } else {
            msg = fmt_lib::format("[*** LOG ERROR ***] [{}({})] [{}] [{}] {}\n", loc.filename, loc.line, date_buf, origin,
                                  ex.what());
        }
        std::fputs(msg.c_str(), stderr);
    } catch (const std::exception &handler_ex) {
        std::fprintf(stderr, "[*** LOG ERROR ***] [%s] exception during %s handler: %s\n", origin.c_str(), custom_err_handler_ ? "custom" : "default",
                     handler_ex.what());
    } catch (...) {  // catch all exceptions
        std::fprintf(stderr, "[*** LOG ERROR ***] [%s] unknown exception during %s handler\n", origin.c_str(), custom_err_handler_ ? "custom" : "default");
    }
}

void err_helper::handle_unknown_ex(const std::string &origin, const source_loc &loc) noexcept {
    handle_ex(origin, loc, std::runtime_error("unknown exception"));
}

void err_helper::set_err_handler(err_handler handler) {
    std::lock_guard lock(mutex_);
    custom_err_handler_ = std::move(handler);
}

}  // namespace details
SPDLOG_NAMESPACE_END
