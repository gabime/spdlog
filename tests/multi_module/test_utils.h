#pragma once

#include <cstddef>
#include <string>
#include <fstream>
#include <vector>

namespace test_utils {

void prepare_logdir(const std::string& dir_name = "test_logs");

std::string file_contents(const std::string& filename);

std::size_t count_lines(const std::string& filename);

std::size_t count_occurrences(const std::string& filename, const std::string& pattern);

bool file_contains(const std::string& filename, const std::string& pattern);

bool file_does_not_contain(const std::string& filename, const std::string& pattern);

std::vector<std::string> read_all_lines(const std::string& filename);

}
