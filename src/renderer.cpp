#include "renderer.h"

#include <cstddef>
#include <string>
#include <vector>

#include "ast.h"
#include "inline_ast.h"

namespace {


void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();  // 避免无限循环
    }
}

// ---------------------------------------------------------------------------
// HTML 特殊字符转义
//
// 按下表替换下面字符，其余字符原样保留：
//   '&' -> "&amp;"
//   '<' -> "&lt;"
//   '>' -> "&gt;"
//   '"' -> "&quot;"
// ---------------------------------------------------------------------------
std::string escape_html(const std::string& s) {
    std::string res = s;
    replace_all(res, "&", "&amp;");
    replace_all(res, "<", "&lt;");
    replace_all(res, ">", "&gt;");
    replace_all(res, "\"", "&quot;");
    return res;
}

// ---------------------------------------------------------------------------
// 渲染行内节点序列
//
// 遍历 nodes，根据 node->type() 分发；需要节点特有数据时用
// dynamic_cast 向下转型（和测试里的用法一样），拼出对应的 HTML：
//
//   InlineType::Text   -> 转义后的文本
//   InlineType::Code   -> "<code>" + 转义文本 + "</code>"
//   InlineType::Bold   -> "<strong>" + 【递归渲染 children】 + "</strong>"
//   InlineType::Italic -> "<em>" + 【递归渲染 children】 + "</em>"
//   InlineType::Link   -> "<a href=\"" + escape(url) + "\">"
//                         + 【递归渲染 children】 + "</a>"
//   InlineType::Image  -> "<img src=\"" + escape(url) + "\" alt=\""
//                         + escape(alt) + "\" />"
// ---------------------------------------------------------------------------
std::string render_inlines(const std::vector<InlinePtr>& nodes) {
    std::string res;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        const InlineNode* node = it->get();
        switch (node->type()) {
            case InlineType::Text:
                res += escape_html(dynamic_cast<const TextNode*>(node)->text());
                break;
            case InlineType::Code:
                res += "<code>";
                res += escape_html(dynamic_cast<const CodeNode*>(node)->text());
                res += "</code>";
                break;
            case InlineType::Bold:
                res += "<strong>";
                res += render_inlines(dynamic_cast<const BoldNode*>(node)->children());
                res += "</strong>";
                break;
            case InlineType::Italic:
                res += "<em>";
                res += render_inlines(dynamic_cast<const ItalicNode*>(node)->children());
                res += "</em>";
                break;
            case InlineType::Link:
                res += "<a href=\"";
                res += escape_html(dynamic_cast<const LinkNode*>(node)->url());
                res += "\">";
                res += render_inlines(dynamic_cast<const LinkNode*>(node)->children());
                res += "</a>";
                break;
            case InlineType::Image:
                res += "<img src=\""; 
                res += escape_html(dynamic_cast<const ImageNode*>(node)->url());
                res += "\" alt=\"";
                res += escape_html(dynamic_cast<const ImageNode*>(node)->alt());
                res += "\" />";
                break;
            default:
                break;
        }
    }
    return res;
}

// ---------------------------------------------------------------------------
// 渲染单个块级节点
// ---------------------------------------------------------------------------
std::string render_block(const BlockNode* node) {
    std::string html;

    switch (node->type()) {
        case BlockType::Heading: {
            const HeadingNode* n = dynamic_cast<const HeadingNode*>(node);
            html += "<h" + std::to_string(n->level()) + ">" +
                    render_inlines(n->inlines()) +
                    "</h" + std::to_string(n->level()) + ">\n";
            break;
        }
        case BlockType::Paragraph: {
            const ParagraphNode* n = dynamic_cast<const ParagraphNode*>(node);
            html += "<p>" + render_inlines(n->inlines()) + "</p>\n";
            break;
        }
        case BlockType::HorizontalRule:
            html += "<hr />\n";
            break;
        case BlockType::CodeBlock: {
            const CodeBlockNode* n = dynamic_cast<const CodeBlockNode*>(node);
            html += "<pre><code";
            if (!n->language().empty()) {
                html += " class=\"language-" + escape_html(n->language()) + "\"";
            }
            // 代码内容只做转义，不做行内渲染，换行原样保留
            html += ">" + escape_html(n->content()) + "</code></pre>\n";
            break;
        }
        case BlockType::Blockquote: {
            const BlockquoteNode* n =
                dynamic_cast<const BlockquoteNode*>(node);
            html += "<blockquote><p>" + render_inlines(n->inlines()) +
                    "</p></blockquote>\n";
            break;
        }
        case BlockType::UnorderedList:
        case BlockType::OrderedList: {
            const ListNode* n = dynamic_cast<const ListNode*>(node);
            const std::string tag = n->ordered() ? "ol" : "ul";
            html += "<" + tag + ">\n";
            for (size_t i = 0; i < n->items().size(); ++i) {
                html += "<li>" + render_inlines(n->items()[i]) + "</li>\n";
            }
            html += "</" + tag + ">\n";
            break;
        }
    }

    return html;
}

}  // namespace

std::string render_html(const std::vector<BlockPtr>& blocks) {
    std::string html;
    for (size_t i = 0; i < blocks.size(); ++i) {
        html += render_block(blocks[i].get());
    }
    return html;
}
