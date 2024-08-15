#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>

#include "ast.h"

// HTML 渲染器：遍历块级 AST（块节点内部再遍历行内 AST），输出 HTML。
//
//   原始文本 -> lexer -> 块级 AST -> 行内 AST -> HTML
//
// render_html 只输出正文片段（<h1>/<p>/<pre> 等，不含 <html> 外壳），
// 便于单元测试断言与管道拼接；需要完整文档时用 wrap_html_document 包装。
std::string render_html(const std::vector<BlockPtr>& blocks);

// 把正文片段包装为完整的 HTML5 文档：
//   - body_html 原样放入 <body> 内的 .markdown-body 容器
//   - css 不为空时原样内嵌到 <head> 的 <style> 中（导出的 HTML 自包含，
//     不依赖外部 css 文件），为空则不生成 <style>
//   - title 作为文档标题（可为空）
std::string wrap_html_document(const std::string& body_html,
                               const std::string& title,
                               const std::string& css);

#endif  // RENDERER_H
