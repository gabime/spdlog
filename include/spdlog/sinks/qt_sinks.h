// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its childs(QTextBrowser...
// etc) Building and using requires Qt library.
//
// Warning: the qt_sink won't be notified if the target widget is destroyed.
// If the widget's lifetime can be shorter than the logger's one, you should provide some permanent QObject,
// and then use a standard signal/slot.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <QMetaMethod>
#include <QTextEdit>
#include <QPlainTextEdit>

//
// qt_sink class
//
namespace spdlog {
namespace sinks {
template<typename Mutex>
class qt_sink : public base_sink<Mutex>
{
public:
    qt_sink(QObject *qt_object, const std::string &meta_method_name)
    {
        // store the meta method object for later usage
        qt_object_ = qt_object;
        const QMetaObject *metaobject = qt_object_->metaObject();
        qt_object_->dumpObjectInfo();
        int methodIndex = metaobject->indexOfMethod(meta_method_name.c_str());
        if (methodIndex == -1) {
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

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;
} // namespace sinks

//
// Factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_mt(const std::string &logger_name, QTextEdit *qt_object, const std::string &meta_method = "append(QString)")
{
    return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qt_logger_st(const std::string &logger_name, QTextEdit *qt_object, const std::string &meta_method = "append(QString)")
{
    return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method);
}

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
