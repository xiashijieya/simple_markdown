#include "file_util.h"

#include <fstream>
#include <sstream>
#include <string>

bool read_file(const std::string& path, std::string& out) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return false;
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    out = oss.str();
    return true;
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    ofs << content;
    return ofs.good();
}
