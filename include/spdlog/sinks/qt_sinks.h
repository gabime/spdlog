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
#include "spdlog/fmt/bundled/color.h"

#include <QObject>

class QTextEdit;
class QPlainTextEdit;

//
// qt_sink class
//
namespace spdlog {
namespace sinks {
template <typename Mutex>
class qt_sink_color;
template <typename Mutex>
class qt_sink : public base_sink<Mutex> {
    friend class qt_sink_color<Mutex>;
public:
    qt_sink(QObject *qt_object = nullptr, const std::string &meta_method = "") {
        qt_object_ = qt_object;
        meta_method_ = meta_method;
    }

    ~qt_sink() { flush_(); }

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str = string_view_t(formatted.data(), formatted.size());
        QMetaObject::invokeMethod(qt_object_, meta_method_.c_str(), Qt::AutoConnection,
                                  Q_ARG(QString, QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed()));
    }

    void flush_() override {}

private:
    QObject *qt_object_ = nullptr;
    std::string meta_method_;
};


template <typename Mutex>
class qt_sink_color : public qt_sink<Mutex> {
public:
    qt_sink_color(QObject *qt_object = nullptr, bool color_ = true, const std::string &meta_method = "")
    {
        qt_sink<Mutex>::qt_object_ = qt_object;
        qt_sink<Mutex>::meta_method_ = meta_method;
        should_do_colors_ = color_;

        set_color(level::trace, fmt::fg(fmt::color::white));
        set_color(level::debug, fmt::fg(fmt::color::cyan));
        set_color(level::info, fmt::fg(fmt::color::green));
        set_color(level::warn, fmt::fg(fmt::color::orange) | fmt::emphasis::bold);
        set_color(level::err, fmt::fg(fmt::color::red) | fmt::emphasis::bold);
        set_color(level::critical, fmt::fg(fmt::color::white) | fmt::bg(fmt::color::red) | fmt::emphasis::bold);
        set_color(level::off, reset_);
    }

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

        colors_[color_level] = str;
    }

    void set_color(level::level_enum color_level, string_view_t color)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);

        colors_[color_level] = to_string_(color);
    }

    ~qt_sink_color() { qt_sink<Mutex>::flush_(); }

    const string_view_t reset_ = "\033[m";

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        msg.color_range_start = 0;
        msg.color_range_end = 0;
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);

        if (should_do_colors_ && msg.color_range_end > msg.color_range_start)
        {
            // before color range
            std::string str;
            auto ccode = to_ccode_(colors_[msg.level]);
            auto reset = to_ccode_(reset_);
            str.reserve(formatted.size() + ccode.size());
            str.append(formatted.begin(), formatted.begin() + msg.color_range_start);
            // in color range
            str.append(ccode.begin(), ccode.end());
            str.append(formatted.begin() + msg.color_range_start, formatted.begin() + msg.color_range_end);
            str.append(reset.begin(), reset.end());
            // after color range
            str.append(formatted.begin() + msg.color_range_end, formatted.end());
            QMetaObject::invokeMethod(qt_sink<Mutex>::qt_object_, qt_sink<Mutex>::meta_method_.c_str(), Qt::AutoConnection,
                                      Q_ARG(QString, QString::fromStdString(str)));
        }
        else // no color
        {
            string_view_t str = string_view_t(formatted.data(), formatted.size());
            QMetaObject::invokeMethod(qt_sink<Mutex>::qt_object_, qt_sink<Mutex>::meta_method_.c_str(), Qt::AutoConnection,
                                      Q_ARG(QString, QString::fromUtf8(str.data(), static_cast<int>(str.size()))));
        }
    }

private:
    string_view_t to_ccode_(const string_view_t &color_code){ return string_view_t(color_code.data(), color_code.size()); }
    static std::string to_string_(const string_view_t &sv) { return std::string(sv.data(), sv.size()); }

private:
    bool should_do_colors_;
    std::array<std::string, level::n_levels> colors_;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;

using qt_sink_color_mt = qt_sink_color<std::mutex>;
using qt_sink_color_st = qt_sink_color<spdlog::details::null_mutex>;
} // namespace sinks

//
// Factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_mt(const std::string &logger_name, QTextEdit* qt_object, const std::string &meta_method = "append") {
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_st(const std::string &logger_name, QTextEdit* qt_object, const std::string &meta_method = "append") {
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_mt(const std::string &logger_name, QPlainTextEdit* qt_object , const std::string &meta_method = "appendPlainText") {
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_st(const std::string &logger_name, QPlainTextEdit* qt_object, const std::string &meta_method = "appendPlainText") {
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

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
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_mt(const std::string &logger_name, QTextEdit* qt_object, bool color, const std::string &meta_method = "append") {
    return Factory::template create<sinks::qt_sink_color_mt>(logger_name, qt_object, color, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_st(const std::string &logger_name, QTextEdit* qt_object, bool color, const std::string &meta_method = "append") {
    return Factory::template create<sinks::qt_sink_color_st>(logger_name, qt_object, color, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_mt(const std::string &logger_name, QPlainTextEdit* qt_object , bool color, const std::string &meta_method = "appendPlainText") {
    return Factory::template create<sinks::qt_sink_color_mt>(logger_name, qt_object, color, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_st(const std::string &logger_name, QPlainTextEdit* qt_object, bool color, const std::string &meta_method = "appendPlainText") {
    return Factory::template create<sinks::qt_sink_color_st>(logger_name, qt_object, color, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_mt(const std::string &logger_name, QObject* qt_object,  bool color, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_color_mt>(logger_name, qt_object, color, meta_method);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_color_st(const std::string &logger_name, QObject* qt_object, bool color, const std::string &meta_method) {
    return Factory::template create<sinks::qt_sink_color_st>(logger_name, qt_object, color, meta_method);
}
} // namespace spdlog
