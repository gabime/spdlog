// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its children(QTextBrowser...etc)
// Building and using requires Qt library.
// Recommended: Use the `qt_color_sink` to get nice coloring per level
// For example
//    auto edit = new QTextEdit();
//    setCentralWidget(edit);
//    auto logger = spdlog::qt_color_logger_mt("my_logger", my_edit_widget);
//
// Warning: those sinks won't be notified if the target widget is destroyed.
// If the widget's lifetime can be shorter than the logger's one, you should provide some permanent QObject,
// and then use a standard signal/slot.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <array>

#include <QMetaMethod>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QColor>

namespace spdlog {
namespace sinks {
template<typename Mutex>
class qt_sink : public base_sink<Mutex>
{
public:
    // qt object is the object that receives the log messages (e.g QPlainTextEdit or QTextEdit)
    // meta_method_name is the name of the slot to be called on the qt_object for every log message (e.g "append(QString)").
    qt_sink(QObject *qt_object, const std::string &meta_method_name)
    {
        // store the meta method object for later usage
        qt_object_ = qt_object;
        auto *metaobject = qt_object_->metaObject();
        auto methodIndex = metaobject->indexOfMethod(meta_method_name.c_str());
        if (methodIndex == -1)
        {
            throw_spdlog_ex("qt_sink: qt_object does not have meta_method " + meta_method_name);
        }
        meta_method_ = metaobject->method(methodIndex);
    }

    ~qt_sink()
    {
        flush_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str = string_view_t(formatted.data(), formatted.size());
        auto payload = QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed();
        meta_method_.invoke(qt_object_, Qt::AutoConnection, Q_ARG(QString, payload));
    }

    void flush_() override {}

private:
    QObject *qt_object_ = nullptr;
    QMetaMethod meta_method_;
};

// QT color sink to QTextEdit.
// Color location is determined by the sink log pattern like in the rest of spdlog sinks.
// Colors can be modified if needed using sink->set_color(level, qtTextCharFormat).
// Note: Only ascii (latin1) is supported by this sink.
template<typename Mutex>
class qt_color_sink : public base_sink<Mutex>
{
public:
    qt_color_sink(QTextEdit *qt_text_edit)
        : qt_text_edit_(qt_text_edit)
    {
        if (!qt_text_edit_)
        {
            throw_spdlog_ex("qt_color_text_sink: text_edit is null");
        }
        default_color_ = qt_text_edit_->currentCharFormat();
        // set colors
        QTextCharFormat format;
        // trace
        format.setForeground(Qt::gray);
        colors_.at(level::trace) = format;
        // debug
        format.setForeground(Qt::cyan);
        colors_.at(level::debug) = format;
        // info
        format.setForeground(Qt::green);
        colors_.at(level::info) = format;
        // warn
        format.setForeground(Qt::yellow);
        colors_.at(level::warn) = format;
        // err
        format.setForeground(Qt::red);
        colors_.at(level::err) = format;
        // critical
        format.setForeground(Qt::white);
        format.setBackground(Qt::red);
        colors_.at(level::critical) = format;
    }

    ~qt_color_sink()
    {
        flush_();
    }

    void set_default_color(QTextCharFormat format)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        default_color_ = format;
    }

    void set_level_color(level::level_enum color_level, QTextCharFormat format)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        colors_.at(static_cast<size_t>(color_level)) = format;
    }

    QTextCharFormat &get_level_color(level::level_enum color_level)
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return colors_.at(static_cast<size_t>(color_level));
    }

    QTextCharFormat &get_default_color()
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return default_color_;
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str = string_view_t(formatted.data(), formatted.size());

        // apply the color to the color range in the formatted message.
        auto payload = QString::fromLatin1(str.data(), static_cast<int>(str.size()));
        if (msg.color_range_end > msg.color_range_start)
        {
            QTextCursor cursor(qt_text_edit_->document());
            cursor.movePosition(QTextCursor::End);

            // insert the text before the color range
            cursor.setCharFormat(default_color_);
            cursor.insertText(payload.left(msg.color_range_start));

            // insert the colorized text
            auto color = colors_.at(static_cast<size_t>(msg.level));
            cursor.setCharFormat(color);
            cursor.insertText(payload.mid(msg.color_range_start, msg.color_range_end - msg.color_range_start));

            // insert the text after the color range with default format
            cursor.setCharFormat(default_color_);
            cursor.insertText(payload.mid(msg.color_range_end));
        }
        else // no color range
        {
            qt_text_edit_->append(payload.trimmed());
        }
    }

    void flush_() override {}
    QTextEdit *qt_text_edit_;
    QTextCharFormat default_color_;
    std::array<QTextCharFormat, level::n_levels> colors_;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;
using qt_color_sink_mt = qt_color_sink<std::mutex>;
using qt_color_sink_st = qt_color_sink<spdlog::details::null_mutex>;
} // namespace sinks

//
// Factory functions
//

// create logger using QTextEdit object
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_mt(
    const std::string &logger_name, QTextEdit *qt_object, const std::string &meta_method = "append(QString)")
{
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_st(
    const std::string &logger_name, QTextEdit *qt_object, const std::string &meta_method = "append(QString)")
{
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

// create logger using QPlainTextEdit object
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_mt(
    const std::string &logger_name, QPlainTextEdit *qt_object, const std::string &meta_method = "appendPlainText(QString)")
{
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_st(
    const std::string &logger_name, QPlainTextEdit *qt_object, const std::string &meta_method = "appendPlainText(QString)")
{
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

// create color qt logger using QTextEdit object
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_color_logger_mt(const std::string &logger_name, QTextEdit *qt_object)
{
    return Factory::template create<sinks::qt_color_sink_mt>(logger_name, qt_object);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_color_logger_st(const std::string &logger_name, QTextEdit *qt_object)
{
    return Factory::template create<sinks::qt_color_sink_st>(logger_name, qt_object);
}

// create logger with other QObject object
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_mt(const std::string &logger_name, QObject *qt_object, const std::string &meta_method)
{
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_st(const std::string &logger_name, QObject *qt_object, const std::string &meta_method)
{
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}
} // namespace spdlog
