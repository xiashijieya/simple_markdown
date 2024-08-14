#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

#include "line.h"

// 词法分析（lexing）：把 Markdown 原始文本拆成“带类型的行”序列。
//
// 处理流程：
//   原始文本
//     -- split_lines -->  若干字符串行
//     -- classify_line（逐行） --> 若干 LineInfo
//
// 本阶段实现下面三个自由函数。自由函数（而非类）适合这种“输入 -> 输出”、
// 不需要保存状态的小工具。

// 按换行符 '\n' 把 text 切成若干行。
//
// 约定：
//   1. 每一行末尾如果带 '\r'（Windows 的 CRLF 换行残留），要去掉；
//   2. 文本末尾的换行不产生额外空行：
//        "a\nb\n"  ->  {"a", "b"}
//      但行与行之间的空行必须保留：
//        "a\n\nb"  ->  {"a", "", "b"}
//   3. 空字符串输入返回空 vector（size 为 0）。
std::vector<std::string> split_lines(const std::string& text);

// 判断单独一行的类型，返回 LineInfo。
// 识别规则见 lexer.cpp 中的 TODO 说明（按顺序判断，注意优先级）。
LineInfo classify_line(const std::string& line);

// 组合上面两个函数：切行 + 逐行分类。
// 此函数已在 .cpp 中实现，你不需要修改。
std::vector<LineInfo> tokenize(const std::string& text);

#endif  // LEXER_H
