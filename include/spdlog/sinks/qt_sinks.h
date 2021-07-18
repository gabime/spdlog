// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its children(QTextBrowser...
// etc) Building and using requires Qt library.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <QObject>
#include <QPlainTextEdit>
#include <QTextEdit>

namespace _spdlog_p {
namespace _sinks_p {
//
// Private class for QTextEdit and its derivatives
//
class qtextedit_sink_p : public QObject
{
    Q_OBJECT
public:
    qtextedit_sink_p(QTextEdit *textedit = nullptr)
    {
        if (textedit != nullptr)
        {
            textedit_ = textedit;
            connect(this, &qtextedit_sink_p::append_text, textedit_, &QTextEdit::append);
        }
    }

    ~qtextedit_sink_p() {}

    void append(const spdlog::string_view_t &str)
    {
        emit append_text(QString::fromUtf8(str.data(), static_cast<int>(str.size() - 2)));
    }

signals:
    void append_text(const QString &);

private:
    QTextEdit *textedit_ = nullptr;
};

//
// Private class for QPlainTextEdit
//
class qplaintextedit_sink_p : public QObject
{
    Q_OBJECT
public:
    qplaintextedit_sink_p(QPlainTextEdit *textedit = nullptr)
    {
        if (textedit != nullptr)
        {
            textedit_ = textedit;
            connect(this, &qplaintextedit_sink_p::append_text, textedit_, &QPlainTextEdit::appendPlainText);
        }
    }

    ~qplaintextedit_sink_p() {}

    void append(const spdlog::string_view_t &str)
    {
        emit append_text(QString::fromUtf8(str.data(), static_cast<int>(str.size() - 2)));
    }

signals:
    void append_text(const QString &);

private:
    QPlainTextEdit *textedit_ = nullptr;
};
} // namespace _sinks_p
} // namespace _spdlog_p

//
// qtextedit_sink class
//
namespace spdlog {
namespace sinks {
template<typename Mutex>
class qtextedit_sink : public base_sink<Mutex>
{
public:
    qtextedit_sink(QTextEdit *textedit = nullptr)
    {
        if (textedit != nullptr)
        {
            textedit_p = std::make_shared<_spdlog_p::_sinks_p::qtextedit_sink_p>(textedit);
        }
        else
        {
            throw spdlog_ex("Error opening QTextEdit");
        }
    }

    ~qtextedit_sink()
    {
        flush_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str_v = string_view_t(formatted.data(), formatted.size());
        textedit_p->append(str_v);
    }

    void flush_() override {}

private:
    std::shared_ptr<_spdlog_p::_sinks_p::qtextedit_sink_p> textedit_p = nullptr;
};

//
// qplaintextedit_sink class
//
template<typename Mutex>
class qplaintextedit_sink : public base_sink<Mutex>
{
public:
    qplaintextedit_sink(QPlainTextEdit *textedit = nullptr)
    {
        if (textedit != nullptr)
        {
            textedit_p = std::make_shared<_spdlog_p::_sinks_p::qplaintextedit_sink_p>(textedit);
        }
        else
        {
            throw spdlog_ex("Error opening QPlainTextEdit");
        }
    }

    ~qplaintextedit_sink()
    {
        flush_();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str_v = string_view_t(formatted.data(), formatted.size());
        textedit_p->append(str_v);
    }

    void flush_() override {}

private:
    std::shared_ptr<_spdlog_p::_sinks_p::qplaintextedit_sink_p> textedit_p = nullptr;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qtextedit_sink_mt = qtextedit_sink<std::mutex>;
using qtextedit_sink_st = qtextedit_sink<spdlog::details::null_mutex>;

using qplaintextedit_sink_mt = qplaintextedit_sink<std::mutex>;
using qplaintextedit_sink_st = qplaintextedit_sink<spdlog::details::null_mutex>;

} // namespace sinks

//
// Factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qtextedit_logger_mt(const std::string &logger_name, QTextEdit *qtextedit = nullptr)
{
    return Factory::template create<sinks::qtextedit_sink_mt>(logger_name, qtextedit);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qtextedit_logger_st(const std::string &logger_name, QTextEdit *qtextedit = nullptr)
{
    return Factory::template create<sinks::qtextedit_sink_st>(logger_name, qtextedit);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qplaintextedit_logger_mt(const std::string &logger_name, QPlainTextEdit *qplaintextedit = nullptr)
{
    return Factory::template create<sinks::qplaintextedit_sink_mt>(logger_name, qplaintextedit);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> qplaintextedit_logger_st(const std::string &logger_name, QPlainTextEdit *qplaintextedit = nullptr)
{
    return Factory::template create<sinks::qplaintextedit_sink_st>(logger_name, qplaintextedit);
}
} // namespace spdlog
