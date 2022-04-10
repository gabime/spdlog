// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#    include <spdlog/sinks/compress_file_sink.h>
#endif

#include <spdlog/common.h>

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>
#include "compress_file_predicate.h"

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>
#include <regex>
#include <chrono>
#include <dirent.h>

namespace spdlog {
namespace sinks {

template<typename Mutex>
SPDLOG_INLINE compress_file_sink<Mutex>::compress_file_sink(
    filename_t base_filename, std::size_t max_size, std::size_t max_gz_files_size, bool rotate_on_open, const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename))
    , max_single_file_disk_capacity_(max_size)
    , max_gz_files_disk_capacity_(max_gz_files_size)
    , file_helper_{event_handlers}
    , compress_worker_runing_(true)
    , need_compress_(false)
{
    if (max_size == 0)
    {
        throw_spdlog_ex("compress sink constructor: max_size arg cannot be zero");
    }

    if (max_gz_files_size < max_size)
    {
        throw_spdlog_ex("compress sink constructor: max_gz_files_size arg cannot less than max_size");
    }

    filename_t log_dir_name = details::os::dir_name(base_filename_);

    if (!details::os::path_exists(log_dir_name)) {
        details::os::create_dir(log_dir_name);
    }
    
    name_set_t un_gz_files = scan_un_gz_files(log_dir_name.c_str(), base_filename_);
     
    filename_t last_log = get_last_log_name(un_gz_files);
    
    file_helper_.open(last_log);
    current_file_size_ = file_helper_.size(); // expensive. called only once
    
    compress_worker_ptr = details::make_unique<std::thread>([this]() {
        while (compress_worker_runing_) 
        {
            std::unique_lock<std::mutex> lock{need_compress_mutex_};
            need_compress_condition_.wait(lock, [this] {
                return need_compress_ == true;
                });

            if (!compress_worker_runing_) {
                return;
            }
            
            async_compress_rotate_();
            need_compress_= false;
        }
    });

    details::os::sleep_for_millis(10);
    
    if (un_gz_files.size() > 1) {
        need_compress_ = true;
        need_compress_condition_.notify_one();
    }
}

template<typename Mutex>
SPDLOG_INLINE name_set_t compress_file_sink<Mutex>::scan_un_gz_files(const char* dirname, filename_t base_name) {
    name_set_t un_gz_files;
    ::DIR* dp = opendir(dirname);
    if (dp) {
        struct ::dirent* entry;
        struct ::stat statbuf;
        char entry_path[1024] = {0};
        while ((entry = readdir(dp)) != NULL){
            memset(entry_path, 0 , 1024);
            sprintf(entry_path, "%s/%s", dirname, entry->d_name);
            lstat(entry_path, &statbuf);
            filename_t un_gz_file = entry_path;
            if (match_un_gz_filename(un_gz_file)) {
                un_gz_files.insert(un_gz_file);
            }
        }// while
        closedir(dp);
    }
    else {
        printf("Open dir:%s faild!\n", dirname);
    }

    return un_gz_files;
}

template<typename Mutex>
SPDLOG_INLINE std::tuple<name_set_t, name_map_t>  compress_file_sink<Mutex>::scan_files(const char* dirname, filename_t base_name) {
    name_set_t un_gz_files;
    name_map_t gz_files;
    ::DIR* dp = opendir(dirname);
    if (dp) {
        struct ::dirent* entry;
        struct ::stat statbuf;
        char entry_path[1024] = {0};
        while ((entry = readdir(dp)) != NULL){
            memset(entry_path, 0 , 1024);
            sprintf(entry_path, "%s/%s", dirname, entry->d_name);
            lstat(entry_path, &statbuf);
            filename_t un_gz_file = entry_path;
            filename_t gz_file    = entry_path;
            if (match_un_gz_filename(un_gz_file)) {
                un_gz_files.insert(un_gz_file);
            }
            if (match_gz_filename(gz_file)) {
                size_t file_size = details::os::filesize(gz_file.c_str());
                gz_files.insert(std::make_pair(gz_file, file_size));
            }
        }// while
        closedir(dp);
    }
    else {
        printf("Open dir:%s faild!\n", dirname);
    }

    return std::make_tuple(un_gz_files, gz_files);
}

template<typename Mutex>
SPDLOG_INLINE bool compress_file_sink<Mutex>::match_un_gz_filename(const filename_t &filename)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(base_filename_);
    basename.append(".");

    //start with basename
    if (basename.size() > filename.size()) {
        return false;
    }
    if (0 != filename.find(basename)) {
        return false;
    }

    //end with ext
    if (ext.size() > filename.size()) {
        return false;
    }
    if (!std::equal(ext.rbegin(), ext.rend(), filename.rbegin())) {
        return false;
    }

    return true;
}

template<typename Mutex>
SPDLOG_INLINE bool compress_file_sink<Mutex>::match_gz_filename(const filename_t &filename)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(base_filename_);
    basename.append(".");

    //start with basename
    if (basename.size() > filename.size()) {
        return false;
    }
    if (0 != filename.find(basename)) {
        return false;
    }

    //end with .gz
    std::string end_str = ".gz";
    if (end_str.size() > filename.size()) {
        return false;
    }
    if (!std::equal(end_str.rbegin(), end_str.rend(), filename.rbegin())) {
        return false;
    }

    return true;
}

// calc filename according to index and file extension if exists.
// e.g. calc_gz_filename("logs/mylog.txt, 3) => "logs/mylog.3.gz".
template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::calc_gz_filename(const filename_t &filename, std::size_t index)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt_lib::format(SPDLOG_FILENAME_T("{}.{}.gz"), basename, index);
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::calc_filename(const filename_t &filename, std::size_t index)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt_lib::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
}

///HUD/log.1.txt => 1
template<typename Mutex>
SPDLOG_INLINE std::size_t compress_file_sink<Mutex>::extract_num_by_filename(const filename_t &filename)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    
    auto index = basename.rfind('.');
    std::string number_str = basename.substr(index+1);
    return std::stoul(number_str);
}

///HUD/log.1.gz => 1
template<typename Mutex>
SPDLOG_INLINE std::size_t compress_file_sink<Mutex>::extract_num_by_gz_filename(const filename_t &filename)
{
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    
    auto index = basename.rfind('.');
    std::string number_str = basename.substr(index+1);
    return std::stoul(number_str);
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt) => "logs/mylog.20220222022222.txt".
template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::timestamp_filename(const filename_t &filename)
{
    auto tm_time = details::os::localtime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    char date_buf[64] = {0};
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d_%H-%M-%S", &tm_time);
    std::string timestamp = date_buf;

    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt_lib::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, timestamp, ext);;
}

template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::filename()
{
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return file_helper_.filename();
}

template<typename Mutex>
SPDLOG_INLINE void compress_file_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    auto new_size = current_file_size_ + formatted.size();

    // rotate if the new estimated file size exceeds max size.
    // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
    // we only check the real size when new_size > max_size_ because it is relatively expensive.
    if (new_size > max_single_file_disk_capacity_)
    {
        file_helper_.flush();
        if (file_helper_.size() > 0)
        {
            quickly_rotate_();
            new_size = formatted.size();
        }
    }
    file_helper_.write(formatted);
    current_file_size_ = new_size;
}

template<typename Mutex>
SPDLOG_INLINE void compress_file_sink<Mutex>::flush_()
{
    file_helper_.flush();
}

// Rotate files:
// log.txt -> log.1.txt
template<typename Mutex>
SPDLOG_INLINE void compress_file_sink<Mutex>::quickly_rotate_()
{
    using details::os::filename_to_str;
    using details::os::path_exists;
    name_set_t un_gz_files = scan_un_gz_files(details::os::dir_name(base_filename_).c_str(), base_filename_);    
    filename_t new_log_file = new_log(un_gz_files);
    file_helper_.open(new_log_file, true);
    
    need_compress_ = true;
    need_compress_condition_.notify_one();
}

// Rotate files:
// log.3.txt.gz -> delete
// log.2.txt.gz -> log.3.txt.gz
// log.1.txt.gz -> log.2.txt.gz
// log.1.txt -> log.1.txt.gz
template<typename Mutex>
SPDLOG_INLINE void compress_file_sink<Mutex>::async_compress_rotate_()
{
    using details::os::filename_to_str;
    using details::os::path_exists;
    
    name_set_t un_gz_files;
    name_map_t gz_files;

    std::tie(un_gz_files, gz_files) =scan_files(details::os::dir_name(base_filename_).c_str(), base_filename_);
    filename_t last_ungz_log = get_last_log_name(un_gz_files);
    filename_t need_compress_file;
    while (!(need_compress_file = get_first_log_name(un_gz_files)).empty() && 
            (need_compress_file != last_ungz_log /*make sure not delete the file which is opening and writing.*/)) {
        un_gz_files.erase(need_compress_file);
        
        if (path_exists(need_compress_file)) {
            filename_t gz_file = calc_gz_filename(base_filename_, extract_num_by_filename(need_compress_file));
            // try to delete the gz_file file in case it already exists.
            (void)details::os::remove(gz_file);
            std::string tar_command;
            tar_command.append("gzip -cf ").append(need_compress_file).append(" > ").append(gz_file);
            std::system(tar_command.c_str());
            size_t file_size = details::os::filesize(gz_file.c_str());
            gz_files.insert(std::make_pair(gz_file, file_size));
            //very import!   make sure not delete the file which is opening and writing.
            (void)details::os::remove(need_compress_file);
        }
    }
        
    remove_old_ifneed_(gz_files, last_ungz_log);
}

// delete the target if exists, and rename the src file  to target
// return true on success, false otherwise.
template<typename Mutex>
SPDLOG_INLINE bool compress_file_sink<Mutex>::rename_file_(const filename_t &src_filename, const filename_t &target_filename)
{   
    // try to delete the target file in case it already exists.
    (void)details::os::remove(target_filename);
    return details::os::rename(src_filename, target_filename) == 0;
}

template<typename Mutex>
SPDLOG_INLINE void compress_file_sink<Mutex>::remove_old_ifneed_(name_map_t& gz_files, filename_t& last_ungz_log)
{
    size_t current_gz_files_size = calc_gz_files_szie(gz_files);
    if (current_gz_files_size > max_gz_files_disk_capacity_) {
        while (current_gz_files_size > max_gz_files_disk_capacity_*0.9) {
            if (!gz_files.empty()) {
                std::map<filename_t, std::size_t>::iterator first_it = get_first_gz(gz_files, last_ungz_log);
                (void)details::os::remove(first_it->first);
                current_gz_files_size -= first_it->second;
                gz_files.erase(first_it);
            }
            else {
                //error
                return;
            } 
        }
    } 
}

template<typename Mutex>
SPDLOG_INLINE name_map_t::iterator compress_file_sink<Mutex>::get_first_gz(name_map_t& gz_files,const filename_t& last_ungz_log) {
    if (gz_files.empty()) {
        return gz_files.end();
    }

    if (gz_files.size() == 1) {
        return gz_files.begin();
    } 

    /*The disadvantage of this algorithm is that out-of-order logs cannot be dynamically adjusted*/     
    // size_t max_num = extract_num_by_gz_filename(std::prev(gz_files.end())->first);
    // size_t min_num = extract_num_by_gz_filename(gz_files.begin()->first);
    // if (max_num >= max_file_serial_number_ && min_num < max_gz_files_count_) {
    //     name_map_t::iterator upper_it = gz_files.upper_bound(calc_gz_filename(base_filename_, max_gz_files_count_));
    //     if (upper_it != gz_files.end() && upper_it !=gz_files.begin()) {
    //         return upper_it;
    //     }
    // }

    //Find the farthest in a 65535 size ring. for example 1,2,3,7,8: 7 is the farthest for 6
    size_t last_num = extract_num_by_filename(last_ungz_log);
    name_map_t::iterator upper_it = gz_files.lower_bound(calc_gz_filename(base_filename_, last_num));
    if (upper_it != gz_files.end() && upper_it !=gz_files.begin()) {
        return upper_it;
    }

    return gz_files.begin();
}

//very import!   make sure not delete the file which is open and writing.
template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::get_first_log_name(name_set_t& un_gz_files) {
    if (un_gz_files.size() < 2) {
        return filename_t{};
    }
    
    auto it = std::prev(un_gz_files.end());
    size_t max_num = extract_num_by_filename(*it);
    size_t min_num = extract_num_by_filename(*(un_gz_files.begin()));
    if (max_num >= max_file_serial_number_ && min_num < max_gz_files_count_) {
        auto upper_it = un_gz_files.upper_bound(calc_filename(base_filename_, max_gz_files_count_));
        if (upper_it != un_gz_files.end() && upper_it !=un_gz_files.begin()) {
            return *upper_it;
        }
    }

    return *(un_gz_files.begin());
}

//very import!   make sure not delete the file which is open and writing.
template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::get_last_log_name(name_set_t& un_gz_files) {
    if (un_gz_files.empty()) {
        filename_t file = calc_filename(base_filename_, 0);
        return file;
    }

    if (un_gz_files.size() == 1) {
        auto it = un_gz_files.begin();
        return *it;
    }
    
    auto it = std::prev(un_gz_files.end());
    size_t max_num = extract_num_by_filename(*it);
    size_t min_num = extract_num_by_filename(*(un_gz_files.begin()));
    if (max_num >= max_file_serial_number_ && min_num < max_gz_files_count_) {
        auto lower_it = un_gz_files.lower_bound(calc_filename(base_filename_, max_gz_files_count_));
        if (lower_it != un_gz_files.end() && lower_it !=un_gz_files.begin()) {
            return *--lower_it;
        }
    }

    return *it;
}

template<typename Mutex>
SPDLOG_INLINE filename_t compress_file_sink<Mutex>::new_log(name_set_t& un_gz_files) {
    filename_t last_log = get_last_log_name(un_gz_files);
    size_t num = extract_num_by_filename(last_log);
    if (num < max_file_serial_number_) {
        filename_t new_file = calc_filename(base_filename_, num + 1);
        return new_file;
    }
    else {
        filename_t new_file = calc_filename(base_filename_, 0);
        return new_file;
    }
}

template<typename Mutex>
SPDLOG_INLINE std::size_t compress_file_sink<Mutex>::calc_gz_files_szie(const name_map_t& gz_files) {
    std::size_t total_size = 0;
    for (const name_map_t::value_type& pair : gz_files) {
        total_size += pair.second;
    }

    return total_size;
}


} // namespace sinks
} // namespace spdlog
