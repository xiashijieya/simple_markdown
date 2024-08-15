#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>

#include "ast.h"

// HTML 渲染器：遍历块级 AST（块节点内部再遍历行内 AST），输出 HTML。
//
//   原始文本 -> lexer -> 块级 AST -> 行内 AST -> HTML
std::string render_html(const std::vector<BlockPtr>& blocks);

#endif  // RENDERER_H
