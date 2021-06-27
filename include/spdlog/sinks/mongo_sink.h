/*****************************************************************************
 * mongo_sink.h
 *
 * Copyright 2021 mguludag
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/
#pragma once
#include "spdlog/common.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/sink.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/view_or_value.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include <mutex>

namespace spdlog {
namespace sinks {
class mongo_sink : public sink {
public:
  mongo_sink(const std::string &db_name, const std::string &collection_name,
             const std::string &uri = "mongodb://localhost:27017") {
    try {
      client_ = std::make_unique<mongocxx::client>(mongocxx::uri{uri});
      db_name_ = db_name;
      coll_name_ = collection_name;
      set_pattern("%v");
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
      throw spdlog_ex("Error opening database");
    }
  }

  void set_pattern(const std::string &pattern) override {
    formatter_ = std::unique_ptr<spdlog::pattern_formatter>(
        new spdlog::pattern_formatter(pattern, pattern_time_type::local, ""));
  }

  void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) {}

  ~mongo_sink() { flush(); }

  void flush() override {}

  void log(const details::log_msg &msg) override {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    if (client_ != nullptr) {
      memory_buf_t formatted;
      formatter_->format(msg, formatted);
      auto doc = document{}
                 << "timestamp" << bsoncxx::types::b_date(msg.time) << "level"
                 << level::to_string_view(msg.level).data() << "message"
                 << std::string(formatted.begin(), formatted.end())
                 << "logger_name"
                 << std::string(msg.logger_name.begin(), msg.logger_name.end())
                 << "thread_id" << static_cast<int>(msg.thread_id) << finalize;
      std::lock_guard<std::mutex> guard(mtx_);
      client_->database(db_name_).collection(coll_name_).insert_one(doc.view());
    }
  }

private:
  static mongocxx::instance instance_;
  std::mutex mtx_;
  std::string db_name_;
  std::string coll_name_;
  std::unique_ptr<mongocxx::client> client_ = nullptr;
  std::unique_ptr<spdlog::pattern_formatter> formatter_;
};
mongocxx::instance mongo_sink::instance_{};
} // namespace sinks
} // namespace spdlog
