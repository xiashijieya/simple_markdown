#ifndef INLINE_PARSER_H
#define INLINE_PARSER_H

#include <string>
#include <vector>

#include "inline_ast.h"

// 行内语法分析：把一段纯文本（段落正文、标题、列表项、引用内容）
// 解析成行内节点序列。
//
// 与块级 parse_blocks 的“逐行吞并”不同，行内解析采用递归下降：
// 遇到开标记（如 "**"）就递归解析其内部，直到对应的闭标记。
// 正因为是递归，嵌套结构（粗体里套斜体、链接文字里套代码）
// 才能自然地表示成节点树。
std::vector<InlinePtr> parse_inlines(const std::string& text);

#endif  // INLINE_PARSER_H
