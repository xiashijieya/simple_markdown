#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <string>

bool read_file(const std::string& path, std::string& out);

bool write_file(const std::string& path, const std::string& content);

#endif  // FILE_UTIL_H
