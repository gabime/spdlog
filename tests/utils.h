#pragma once

#include <cstddef>
#include <string>

std::size_t count_lines(const std::string &filename);

void prepare_logdir();

std::string file_contents(const std::string &filename);

std::size_t count_lines(const std::string &filename);

std::size_t get_filesize(const std::string &filename);

bool ends_with(std::string const &value, std::string const &ending);