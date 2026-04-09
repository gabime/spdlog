#pragma once

#include "spdlog/common.h"
#include "spdlog/details/os.h"
#include <cstddef>
#include <string>
#include <fstream>
#include <vector>

namespace test_utils {

inline spdlog::filename_t path_join(const spdlog::filename_t& dir, const spdlog::filename_t& filename) {
    if (dir.empty()) {
        return filename;
    }
    
    spdlog::filename_t result = dir;
    
    if (!result.empty()) {
        auto last_char = result.back();
        if (last_char != SPDLOG_FILENAME_T('/') && last_char != SPDLOG_FILENAME_T('\\')) {
#ifdef _WIN32
            result += SPDLOG_FILENAME_T('\\');
#else
            result += SPDLOG_FILENAME_T('/');
#endif
        }
    }
    
    result += filename;
    return result;
}

inline std::string filename_to_string(const spdlog::filename_t& filename) {
    return spdlog::details::os::filename_to_str(filename);
}

void prepare_logdir(const std::string& dir_name = "test_logs");

std::string file_contents(const std::string& filename);

std::size_t count_lines(const std::string& filename);

std::size_t count_occurrences(const std::string& filename, const std::string& pattern);

bool file_contains(const std::string& filename, const std::string& pattern);

bool file_does_not_contain(const std::string& filename, const std::string& pattern);

std::vector<std::string> read_all_lines(const std::string& filename);

}
