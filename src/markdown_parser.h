#ifndef MARKDOWN_PARSER_H
#define MARKDOWN_PARSER_H

#include <string>
#include <vector>

#include "ast.h"

// Markdown 解析器：本项目的核心类。
// 它内部经历这样一条处理链：
//   原始文本 -> 词法分析(按行识别) -> 语法树(AST) -> HTML
class MarkdownParser {
public:
    MarkdownParser();
    explicit MarkdownParser(const std::string& markdown_text);

    // 设置/替换待解析的 markdown 文本
    void set_text(const std::string& markdown_text);

    // 解析文本，生成内部语法树
    void parse();

    // 把解析结果渲染为 HTML 字符串
    std::string to_html() const;

    // 访问解析得到的 AST
    const std::vector<BlockPtr>& blocks() const;

private:
    // 成员变量约定：末尾加下划线，避免与普通变量/参数混淆
    std::string markdown_text_;
    std::vector<BlockPtr> blocks_;
};

#endif // MARKDOWN_PARSER_H
