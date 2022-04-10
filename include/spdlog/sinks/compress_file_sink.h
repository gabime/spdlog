// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/synchronous_factory.h>
#include "compress_file_predicate.h"

#include <chrono>
#include <mutex>
#include <string>
#include <map>

namespace spdlog {
namespace sinks {

//
// compress file sink based on size
//
template<typename Mutex>
class compress_file_sink final : public base_sink<Mutex>
{
public:
    compress_file_sink(filename_t base_filename, std::size_t max_size, std::size_t max_gz_files_size, bool rotate_on_open = false,
        const file_event_handlers &event_handlers = {});
    static filename_t calc_filename(const filename_t &filename, std::size_t index);
    filename_t filename();

    ~compress_file_sink() {
        if (!compress_worker_ptr) {
            if (compress_worker_ptr->joinable()){
                compress_worker_runing_ = false;
                {
                    std::lock_guard<std::mutex> lock{need_compress_mutex_}; 
                    need_compress_ = true;
                    need_compress_condition_.notify_one();
                }
                compress_worker_ptr->join();
            }
        }
    }

public:


protected:
    void sink_it_(const details::log_msg &msg) override;
    void flush_() override;

private:
    // Rotate files:
    // log.txt -> log.1.txt.tar.gz
    // log.1.txt.tar.gz -> log.2.txt.tar.gz
    // log.2.txt.tar.gz -> log.3.txt.tar.gz
    // log.3.txt.tar.gz -> delete
    //void rotate_();

    // delete the target if exists, and rename the src file  to target
    // return true on success, false otherwise.
    bool rename_file_(const filename_t &src_filename, const filename_t &target_filename);

    void remove_old_ifneed_(name_map_t& gz_files, filename_t& last_ungz_log);
    void async_compress_rotate_();
    void quickly_rotate_();
    void slow_rotate_();
    filename_t calc_gz_filename(const filename_t &filename, std::size_t index);
    filename_t wait_compress_filename(const filename_t &filename);
    filename_t timestamp_filename(const filename_t &filename);
    std::tuple<name_set_t, name_map_t> scan_files(const char* dirname, filename_t base_name);
    bool match_gz_filename(const filename_t &filename);
    name_set_t scan_un_gz_files(const char* dirname, filename_t base_name);
    bool match_un_gz_filename(const filename_t &filename);
    void add_gz_(const filename_t& gz_file);
    std::size_t extract_num_by_filename(const filename_t &filename);
    std::size_t extract_num_by_gz_filename(const filename_t &filename);
    filename_t get_last_log_name(name_set_t& un_gz_files);
    filename_t get_first_log_name(name_set_t& un_gz_files);
    name_map_t::iterator get_first_gz(name_map_t& gz_files,const filename_t& last_ungz_log);
    filename_t new_log(name_set_t& un_gz_files);
    std::size_t calc_gz_files_szie(const name_map_t& gz_files);

    filename_t base_filename_;
    
    std::size_t max_file_serial_number_ = 65535; 
    std::size_t max_single_file_disk_capacity_;
    std::size_t current_file_size_;

    std::set<filename_t> un_gz_files_;
        
    std::size_t max_gz_files_disk_capacity_;
    std::size_t max_gz_files_count_ = 1024;
    std::atomic_bool need_compress_;
    std::mutex need_compress_mutex_;
    std::condition_variable need_compress_condition_; 
    std::unique_ptr<std::thread> compress_worker_ptr;
    std::atomic_bool compress_worker_runing_;

    details::file_helper file_helper_;
};

using compress_file_sink_mt = compress_file_sink<std::mutex>;
using compress_file_sink_st = compress_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> compress_logger_mt(const std::string &logger_name, const filename_t &filename, size_t max_file_size,
    size_t max_gz_files_size, bool rotate_on_open = false, const file_event_handlers &event_handlers = {})
{
    return Factory::template create<sinks::compress_file_sink_mt>(
        logger_name, filename, max_file_size, max_gz_files_size, rotate_on_open, event_handlers);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> compress_logger_st(const std::string &logger_name, const filename_t &filename, size_t max_file_size,
    size_t max_gz_files_size, bool rotate_on_open = false, const file_event_handlers &event_handlers = {})
{
    return Factory::template create<sinks::compress_file_sink_st>(
        logger_name, filename, max_file_size, max_gz_files_size, rotate_on_open, event_handlers);
}
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "compress_file_sink-inl.h"
#endif
