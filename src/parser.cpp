#include "parser.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"
#include "inline_parser.h"
#include "line.h"

namespace {

BlockPtr parse_code_block(const std::vector<LineInfo>& lines, size_t& pos) {
    const std::string lang = lines[pos].content_;
    ++pos;

    std::string content;
    bool first = true;
    while (pos < lines.size() && lines[pos].type_ != LineType::CodeFence) {
        if (!first) {
            content += '\n';
        }
        content += lines[pos].raw_;
        first = false;
        ++pos;
    }
    if (pos < lines.size()) {
        ++pos;  // 消费结束围栏；若扫描到文件末尾（未闭合）则没有
    }
    // 代码块内容不做行内解析，保持纯文本
    return BlockPtr(new CodeBlockNode(lang, content));
}

BlockPtr parse_paragraph(const std::vector<LineInfo>& lines, size_t& pos) {
    std::string text = lines[pos].content_;
    ++pos;
    while (pos < lines.size() && lines[pos].type_ == LineType::Text) {
        text += ' ';
        text += lines[pos].content_;
        ++pos;
    }
    return BlockPtr(new ParagraphNode(parse_inlines(text)));
}

BlockPtr parse_list(const std::vector<LineInfo>& lines, size_t& pos) {
    const bool ordered = (lines[pos].type_ == LineType::OrderedList);
    const LineType want = ordered ? LineType::OrderedList
                                  : LineType::UnorderedList;
    std::vector<std::vector<InlinePtr>> items;
    items.push_back(parse_inlines(lines[pos].content_));
    ++pos;
    while (pos < lines.size() && lines[pos].type_ == want) {
        items.push_back(parse_inlines(lines[pos].content_));
        ++pos;
    }
    return BlockPtr(new ListNode(ordered, std::move(items)));
}


BlockPtr parse_blockquote(const std::vector<LineInfo>& lines, size_t& pos) {
    std::string text = lines[pos].content_;
    ++pos;
    while (pos < lines.size() && lines[pos].type_ == LineType::Blockquote) {
        text += '\n';
        text += lines[pos].content_;
        ++pos;
    }
    return BlockPtr(new BlockquoteNode(parse_inlines(text)));
}

}  // namespace

std::vector<BlockPtr> parse_blocks(const std::vector<LineInfo>& lines) {
    std::vector<BlockPtr> doc;
    size_t pos = 0;

    while (pos < lines.size()) {
        const LineInfo& line = lines[pos];
        BlockPtr node;

        switch (line.type_) {
            case LineType::Blank:
                ++pos;
                continue;

            case LineType::Heading:
                node.reset(new HeadingNode(line.level_, parse_inlines(line.content_)));
                ++pos;
                break;

            case LineType::HorizontalRule:
                node.reset(new HorizontalRuleNode());
                ++pos;
                break;

            case LineType::CodeFence:
                node = parse_code_block(lines, pos);
                break;

            case LineType::Text:
                node = parse_paragraph(lines, pos);
                break;

            case LineType::UnorderedList:
            case LineType::OrderedList:
                node = parse_list(lines, pos);
                break;

            case LineType::Blockquote:
                node = parse_blockquote(lines, pos);
                break;

            case LineType::Unknown:
            default:

                ++pos;
                break;
        }


        if (node) {
            doc.push_back(std::move(node));
        }
    }

    return doc;
}
