// Copyright(c) 2015-present, Gabi Melman, mguludag and spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for QTextEdit and its childs(QTextBrowser... etc)
// Building and using requires Qt library.
//

#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include "qtextedit_sink_p.h"

namespace spdlog {
namespace sinks {
template <typename Mutex>
class qtextedit_sink : public base_sink<Mutex> {
public:
  qtextedit_sink(QTextEdit *textedit = nullptr) {
    if (textedit != nullptr) {
      textedit_p = std::make_shared<_spdlog_p::_sinks_p::qtextedit_sink_p>(textedit);
    } else {
      throw spdlog_ex("Error opening QTextEdit");
    }
  }

  ~qtextedit_sink() { flush_(); }

protected:
  void sink_it_(const details::log_msg &msg) override {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    auto str = std::string(formatted.begin(), formatted.end() - 2);
    textedit_p->append(str);
  }

  void flush_() override {}

private:
    std::shared_ptr<_spdlog_p::_sinks_p::qtextedit_sink_p> textedit_p = nullptr;
};

#include "spdlog/details/null_mutex.h"
#include <mutex>
using qtextedit_sink_mt = qtextedit_sink<std::mutex>;
using qtextedit_sink_st = qtextedit_sink<spdlog::details::null_mutex>;

} // namespace sinks

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

} // namespace spdlog
