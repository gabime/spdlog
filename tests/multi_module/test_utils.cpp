#include "test_utils.h"
#include "spdlog/spdlog.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

namespace test_utils {

void prepare_logdir(const std::string& dir_name) {
    spdlog::drop_all();
    
#ifdef _WIN32
    std::string cmd = "rmdir /S /Q " + dir_name + " 2>nul";
    system(cmd.c_str());
    cmd = "mkdir " + dir_name;
    system(cmd.c_str());
#else
    std::string cmd = "rm -rf " + dir_name;
    auto rv = system(cmd.c_str());
    (void)rv;
    cmd = "mkdir -p " + dir_name;
    rv = system(cmd.c_str());
    if (rv != 0) {
        throw std::runtime_error("Failed to create directory: " + dir_name);
    }
#endif
}

std::string file_contents(const std::string& filename) {
    std::ifstream ifs(filename, std::ios_base::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
}

std::size_t count_lines(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string line;
    size_t counter = 0;
    while (std::getline(ifs, line)) {
        counter++;
    }
    return counter;
}

std::size_t count_occurrences(const std::string& filename, const std::string& pattern) {
    std::string content = file_contents(filename);
    size_t count = 0;
    size_t pos = 0;
    
    while ((pos = content.find(pattern, pos)) != std::string::npos) {
        count++;
        pos += pattern.length();
    }
    return count;
}

bool file_contains(const std::string& filename, const std::string& pattern) {
    return count_occurrences(filename, pattern) > 0;
}

bool file_does_not_contain(const std::string& filename, const std::string& pattern) {
    return count_occurrences(filename, pattern) == 0;
}

std::vector<std::string> read_all_lines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string line;
    while (std::getline(ifs, line)) {
        lines.push_back(line);
    }
    return lines;
}

}
