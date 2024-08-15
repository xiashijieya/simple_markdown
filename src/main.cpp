#include <iostream>
#include <sstream>
#include <string>

#include "file_util.h"
#include "markdown_parser.h"

namespace {

const char* kUsage =
    "用法: simple_markdown [输入文件] [-o 输出文件]\n"
    "  省略输入文件：从标准输入读取\n"
    "  省略 -o    ：HTML 输出到标准输出\n"
    "  选项: -h, --help  显示本帮助\n"
    "示例:\n"
    "  simple_markdown doc.md\n"
    "  simple_markdown doc.md -o doc.html\n"
    "  cat doc.md | simple_markdown\n";

std::string read_stdin() {
    std::ostringstream ss;
    ss << std::cin.rdbuf();
    return ss.str();
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string input_path;
    std::string output_path;

    // 简单命令行解析：逐个扫描参数。
    // 不以 '-' 开头的第一个参数是输入文件；-o 消耗它后面的一个参数。
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << kUsage;
            return 0;
        }
        if (arg == "-o") {
            if (i + 1 >= argc) {
                std::cerr << "错误: -o 后面需要一个输出文件路径\n\n" << kUsage;
                return 1;
            }
            output_path = argv[++i];
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "错误: 未知选项 " << arg << "\n\n" << kUsage;
            return 1;
        } else if (!input_path.empty()) {
            std::cerr << "错误: 多余的参数 " << arg << "\n\n" << kUsage;
            return 1;
        } else {
            input_path = arg;
        }
    }

    // ---- 1. 读取输入 ---------------------------------------------------
    std::string text;
    if (input_path.empty()) {
        text = read_stdin();
    } else {
        if (!read_file(input_path, text)) {
            std::cerr << "Error: Cannot open the file" << input_path;
            return 1;
        }
    }

    // ---- 2. 解析并渲染 ------------------------------ ------------------
    MarkdownParser parser(text);
    parser.parse();
    std::string html = parser.to_html();

    // ---- 3. 输出结果 ---------------------------------------------------
    if (output_path.empty()) {
        std::cout << html;
    } else {
        if (!write_file(output_path, html)) {
            std::cerr << "Error: Cannot open the file " << output_path;
            return 1;
        }
    }

    return 0;
}
