#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "markdown_parser.h"

std::string read_file(const std::string& path);

int main(int argc, char* argv[]) {
    std::string text;

    if (argc >= 2) {
        // 用法：simple_markdown <文件路径>
        text = read_file(argv[1]);
    } else {
        // 没有传文件名时，从标准输入读取（管道输入）
        std::ostringstream ss;
        ss << std::cin.rdbuf();
        text = ss.str();
    }
    
    MarkdownParser parser(text);
    parser.parse();
    std::cout << parser.to_html();

    return 0;
}

std::string read_file(const std::string& path) {
    std::string text;
    std::ifstream ifs(path);
    if (ifs.is_open()) {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        text = oss.str();
    }
    return text;
}
