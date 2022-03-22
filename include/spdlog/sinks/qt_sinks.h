// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its children(QTextBrowser...
// etc) also any QObject will be ok with QString slot. Building and using requires Qt library.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#if defined(SPDLOG_FMT_EXTERNAL)
#include <fmt/color.h>
#elif !defined(SPDLOG_USE_STD_FORMAT)
#include "spdlog/fmt/bundled/color.h"
#endif

#include <QObject>

//
// qt_sink class
//
namespace spdlog {
namespace sinks {
template <typename Mutex>
class qt_sink : public base_sink<Mutex> {
public:
    qt_sink(QObject *qt_object = nullptr, const std::string &meta_method = "") : qt_object_(qt_object), meta_method_(meta_method_) { }
    ~qt_sink() { flush_(); }

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        QMetaObject::invokeMethod(qt_object_, meta_method_.c_str(), Qt::AutoConnection,
                                  Q_ARG(const QString&, QString::fromUtf8(formatted.data(), formatted.size())));
    }

    void flush_() override {}

private:
    QObject *qt_object_;
    std::string meta_method_;
};

#if !defined(SPDLOG_USE_STD_FORMAT)
template <typename Mutex>
class qt_sink_color : public base_sink<Mutex> {
public:
    qt_sink_color(QObject *qt_object = nullptr, const std::string &meta_method = "") : qt_object_(qt_object), meta_method_(meta_method)
    {
        set_color(level::trace, fmt::fg(fmt::color::white));
        set_color(level::debug, fmt::fg(fmt::color::cyan));
        set_color(level::info, fmt::fg(fmt::color::green));
        set_color(level::warn, fmt::fg(fmt::color::orange) | fmt::emphasis::bold);
        set_color(level::err, fmt::fg(fmt::color::red) | fmt::emphasis::bold);
        set_color(level::critical, fmt::fg(fmt::color::white) | fmt::bg(fmt::color::red) | fmt::emphasis::bold);
        set_color(level::off, reset_);
    }

    ~qt_sink_color() { flush_(); }

    void set_color(level::level_enum color_level, fmt::text_style color)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        std::string str;
        auto bg = fmt::detail::make_background_color<fmt::detail::char8_type>(color.get_background());
        auto fg = fmt::detail::make_foreground_color<fmt::detail::char8_type>(color.get_foreground());
        auto em = fmt::detail::make_emphasis<fmt::detail::char8_type>(color.get_emphasis());
        str.append(bg.begin(), bg.end());
        str.append(fg.begin(), fg.end());
        str.append(em.begin(), em.end());
        colors_[color_level] = std::move(str);
    }

    void set_color(level::level_enum color_level, string_view_t color)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        colors_[color_level] = to_string_(color);
    }

    void flush_() override {}


    const string_view_t reset_ = "\033[m";

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        msg.color_range_start = 0;
        msg.color_range_end = 0;
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);

        if (msg.color_range_end > msg.color_range_start)
        {
            // before color range
            memory_buf_t colored;
            auto ccode = to_ccode_(colors_[msg.level]);
            auto reset = to_ccode_(reset_);
            colored.append(formatted.begin(), formatted.begin() + msg.color_range_start);
            // in color range
            colored.append(ccode.begin(), ccode.end());
            colored.append(formatted.begin() + msg.color_range_start, formatted.begin() + msg.color_range_end);
            colored.append(reset.begin(), reset.end());
            // after color range
            colored.append(formatted.begin() + msg.color_range_end, formatted.end());
            QMetaObject::invokeMethod(qt_object_, meta_method_.c_str(), Qt::AutoConnection,
                                      Q_ARG(const QString&, QString::fromUtf8(colored.data(), colored.size())));
        }
        else // no color
        {
            QMetaObject::invokeMethod(qt_object_, meta_method_.c_str(), Qt::AutoConnection,
                                      Q_ARG(const QString&, QString::fromUtf8(formatted.data(), formatted.size())));
        }
    }

private:
    string_view_t to_ccode_(const string_view_t &color_code){ return string_view_t(color_code.data(), color_code.size()); }
    static std::string to_string_(const string_view_t &sv) { return std::string(sv.data(), sv.size()); }

private:
    QObject *qt_object_;
    std::string meta_method_;
    std::array<std::string, level::n_levels> colors_;
};
#endif

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;

#if !defined(SPDLOG_USE_STD_FORMAT)
using qt_sink_color_mt = qt_sink_color<std::mutex>;
using qt_sink_color_st = qt_sink_color<spdlog::details::null_mutex>;
#endif
} // namespace sinks

//
// Factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_mt(const std::string &logger_name, QObject* qt_object, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_st(const std::string &logger_name, QObject* qt_object, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

//
// Factory functions (color)
//
#if !defined(SPDLOG_USE_STD_FORMAT)
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_mt(const std::string &logger_name, QObject* qt_object, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_color_mt>(logger_name, qt_object, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_st(const std::string &logger_name, QObject* qt_object, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_color_st>(logger_name, qt_object, meta_method);
}
#endif
} // namespace spdlog
