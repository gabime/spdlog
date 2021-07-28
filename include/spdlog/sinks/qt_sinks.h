// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QPlainTextEdit or QTextEdit and its childs(QTextBrowser...
// etc) Building and using requires Qt library.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include <type_traits>
#include <QTextEdit>
#include <QPlainTextEdit>

//
// qt_sink class
//
namespace spdlog {
namespace sinks {
template <typename Mutex> class qt_sink : public base_sink<Mutex> {
public:
  qt_sink(QObject *qt_object = nullptr, const std::string &meta_method = "") {
    if (qt_object != nullptr) {
      qt_object_ = qt_object;
      meta_method_ = meta_method;
    } else {
      throw spdlog_ex("Error opening qt sink");
    }
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

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qt_sink_mt = qt_sink<std::mutex>;
using qt_sink_st = qt_sink<spdlog::details::null_mutex>;
} // namespace sinks

//
// Factory functions
//
template <typename T, typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_mt(const std::string &logger_name, T *qt_object = nullptr, const std::string &meta_method = "") {
  std::string meta_method_ = meta_method;
  if (std::is_base_of<QTextEdit, T>::value)
    meta_method_ = "append";
  else if (std::is_same<T, QPlainTextEdit>::value)
    meta_method_ = "appendPlainText";
  return Factory::template create<sinks::qt_sink_mt>(logger_name, qt_object, meta_method_);
}

template <typename T, typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger>
qt_logger_st(const std::string &logger_name, T *qt_object = nullptr, const std::string &meta_method = "") {
  std::string meta_method_ = meta_method;
  if (std::is_base_of<QTextEdit, T>::value)
    meta_method_ = "append";
  else if (std::is_same<T, QPlainTextEdit>::value)
    meta_method_ = "appendPlainText";
  return Factory::template create<sinks::qt_sink_st>(logger_name, qt_object, meta_method_);
}
} // namespace spdlog
