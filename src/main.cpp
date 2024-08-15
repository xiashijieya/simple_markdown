#include <iostream>
#include <sstream>
#include <string>

#include "file_util.h"
#include "markdown_parser.h"
#include "renderer.h"

namespace {

// 默认主题：相对于程序运行时的工作目录。分发时把 theme/ 目录和
// simple_markdown 可执行文件放在一起，在该目录下运行即可。
const char* kDefaultThemePath = "theme/default.css";

const char* kUsage =
    "用法: simple_markdown [输入文件] [-o 输出文件] [-t 主题文件]\n"
    "  省略输入文件：从标准输入读取\n"
    "  省略 -o    ：HTML 片段输出到标准输出（不含 html 外壳与样式）\n"
    "  使用 -o    ：导出完整 HTML 文档，并把主题 CSS 内嵌到 <style> 中\n"
    "  省略 -t    ：使用内置默认主题 theme/default.css\n"
    "  选项: -h, --help  显示本帮助\n"
    "示例:\n"
    "  simple_markdown doc.md\n"
    "  simple_markdown doc.md -o doc.html\n"
    "  simple_markdown doc.md -o doc.html -t my.css\n"
    "  cat doc.md | simple_markdown\n";

std::string read_stdin() {
    std::ostringstream ss;
    ss << std::cin.rdbuf();
    return ss.str();
}

// 取文件路径的“主干”作为文档标题：去掉目录部分与最后一个扩展名。
//   "out/doc.html" -> "doc"
//   "README"       -> "README"
std::string file_stem(const std::string& path) {
    size_t begin = path.find_last_of("/\\");
    begin = (begin == std::string::npos) ? 0 : begin + 1;
    size_t end = path.find_last_of('.');
    if (end == std::string::npos || end < begin) {
        end = path.size();
    }
    return path.substr(begin, end - begin);
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string input_path;
    std::string output_path;
    std::string theme_path = kDefaultThemePath;

    // 简单命令行解析：逐个扫描参数。
    // 不以 '-' 开头的第一个参数是输入文件；-o/-t 各消耗后面的一个参数。
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::cout << kUsage;
            return 0;
        }
        if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                std::cerr << "错误: " << arg << " 后面需要一个输出文件路径\n\n"
                          << kUsage;
                return 1;
            }
            output_path = argv[++i];
        } else if (arg == "-t" || arg == "--theme") {
            if (i + 1 >= argc) {
                std::cerr << "错误: " << arg << " 后面需要一个主题文件路径\n\n"
                          << kUsage;
                return 1;
            }
            theme_path = argv[++i];
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

    // ---- 2. 解析并渲染 -------------------------------------------------
    MarkdownParser parser(text);
    parser.parse();
    std::string html = parser.to_html();

    // ---- 3. 输出结果 ---------------------------------------------------
    if (output_path.empty()) {
        std::cout << html;
    } else {
        std::string css;
        if (!read_file(theme_path, css)) {
            std::cerr << "警告: 无法读取主题文件 " << theme_path
                      << "，导出的 HTML 将不包含样式\n";
        }
        const std::string document =
            wrap_html_document(html, file_stem(output_path), css);
        if (!write_file(output_path, document)) {
            std::cerr << "Error: Cannot open the file " << output_path;
            return 1;
        }
    }

    return 0;
}
