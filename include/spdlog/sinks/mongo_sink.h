// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

//
// Custom sink for mongodb
// Building and using requires mongocxx library.
// For building mongocxx library check the url below
// http://mongocxx.org/mongocxx-v3/installation/
//

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/view_or_value.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include "../common.h"
#include "../details/log_msg.h"
#include "../details/synchronous_factory.h"
#include "./base_sink.h"

namespace spdlog {
namespace sinks {
template <typename Mutex>
class mongo_sink : public base_sink<Mutex> {
public:
    mongo_sink(const std::string &db_name,
               const std::string &collection_name,
               const std::string &uri = "mongodb://localhost:27017") try
        : mongo_sink(std::make_shared<mongocxx::instance>(), db_name, collection_name, uri) {
    } catch (const std::exception &e) {
        throw_spdlog_ex(fmt_lib::format("Error opening database: {}", e.what()));
    }

    mongo_sink(std::shared_ptr<mongocxx::instance> instance,
               const std::string &db_name,
               const std::string &collection_name,
               const std::string &uri = "mongodb://localhost:27017")
        : instance_(std::move(instance)),
          db_name_(db_name),
          coll_name_(collection_name) {
        try {
            client_ = spdlog::std::make_unique<mongocxx::client>(mongocxx::uri{uri});
        } catch (const std::exception &e) {
            throw_spdlog_ex(fmt_lib::format("Error opening database: {}", e.what()));
        }
    }

    ~mongo_sink() { flush_(); }

protected:
    void sink_it_(const details::log_msg &msg) override {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;

        if (client_ != nullptr) {
            auto doc = document{} << "timestamp" << bsoncxx::types::b_date(msg.time) << "level"
                                  << level::to_string_view(msg.log_level).data() << "level_num" << msg.log_level << "message"
                                  << std::string(msg.payload.begin(), msg.payload.end()) << "logger_name"
                                  << std::string(msg.logger_name.begin(), msg.logger_name.end()) << "thread_id"
                                  << static_cast<int>(msg.thread_id) << finalize;
            client_->database(db_name_).collection(coll_name_).insert_one(doc.view());
        }
    }

    void flush_() override {}

private:
    std::shared_ptr<mongocxx::instance> instance_;
    std::string db_name_;
    std::string coll_name_;
    std::unique_ptr<mongocxx::client> client_ = nullptr;
};

#include <mutex>

#include "../details/null_mutex.h"
using mongo_sink_mt = mongo_sink<std::mutex>;
using mongo_sink_st = mongo_sink<spdlog::details::null_mutex>;

}  // namespace sinks

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> mongo_logger_mt(const std::string &logger_name,
                                               const std::string &db_name,
                                               const std::string &collection_name,
                                               const std::string &uri = "mongodb://localhost:27017") {
    return Factory::template create<sinks::mongo_sink_mt>(logger_name, db_name, collection_name, uri);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> mongo_logger_st(const std::string &logger_name,
                                               const std::string &db_name,
                                               const std::string &collection_name,
                                               const std::string &uri = "mongodb://localhost:27017") {
    return Factory::template create<sinks::mongo_sink_st>(logger_name, db_name, collection_name, uri);
}

}  // namespace spdlog
