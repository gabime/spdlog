// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its childs(QTextBrowser... etc)
// Building and using requires Qt library.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <QTextEdit>
#include <QPlainTextEdit>

//
// qtextedit_sink class
//
namespace spdlog {
namespace sinks {
template <typename Mutex>
class qtextedit_sink : public base_sink<Mutex> {
public:
  qtextedit_sink(QTextEdit *textedit = nullptr) {
    if (textedit != nullptr) {
        qtextedit_ = textedit;
    } else {
      throw spdlog_ex("Error opening QTextEdit");
    }
  }

  ~qtextedit_sink() { flush_(); }

protected:
  void sink_it_(const details::log_msg &msg) override {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    string_view_t str = string_view_t(formatted.data(), formatted.size());
    QMetaObject::invokeMethod(qtextedit_,"append", Qt::AutoConnection, Q_ARG(QString, QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed()));
  }

  void flush_() override {}

private:
    QTextEdit* qtextedit_ = nullptr;
};

//
// qplaintextedit_sink class
//
template <typename Mutex>
class qplaintextedit_sink : public base_sink<Mutex> {
public:
    qplaintextedit_sink(QPlainTextEdit *textedit = nullptr) {
        if (textedit != nullptr) {
            qplaintextedit_ = textedit;
        } else {
            throw spdlog_ex("Error opening QPlainTextEdit");
        }
    }

    ~qplaintextedit_sink() { flush_(); }

protected:
    void sink_it_(const details::log_msg &msg) override {
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        string_view_t str = string_view_t(formatted.data(), formatted.size());
        QMetaObject::invokeMethod(qplaintextedit_, "appendPlainText", Qt::AutoConnection, Q_ARG(QString, QString::fromUtf8(str.data(), static_cast<int>(str.size())).trimmed()));
    }

    void flush_() override {}

private:
    QPlainTextEdit* qplaintextedit_ = nullptr;
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
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qtextedit_logger_mt(const std::string &logger_name,
                    QTextEdit *qtextedit = nullptr) {
  return Factory::template create<sinks::qtextedit_sink_mt>(logger_name,
                                                            qtextedit);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qtextedit_logger_st(const std::string &logger_name,
                    QTextEdit *qtextedit = nullptr) {
  return Factory::template create<sinks::qtextedit_sink_st>(logger_name,
                                                            qtextedit);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qplaintextedit_logger_mt(const std::string &logger_name,
                    QPlainTextEdit *qplaintextedit = nullptr) {
    return Factory::template create<sinks::qplaintextedit_sink_mt>(logger_name,
                                                              qplaintextedit);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qplaintextedit_logger_st(const std::string &logger_name,
                    QPlainTextEdit *qplaintextedit = nullptr) {
    return Factory::template create<sinks::qplaintextedit_sink_st>(logger_name,
                                                              qplaintextedit);
}
} // namespace spdlog
