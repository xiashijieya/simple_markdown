#include "markdown_parser.h"

#include <string>
#include <utility>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "renderer.h"

MarkdownParser::MarkdownParser() {
}

MarkdownParser::MarkdownParser(const std::string& markdown_text)
    : markdown_text_(markdown_text) {
}

void MarkdownParser::set_text(const std::string& markdown_text) {
    markdown_text_ = markdown_text;
}

void MarkdownParser::parse() {
    blocks_ = parse_blocks(tokenize(markdown_text_));
}

std::string MarkdownParser::to_html() const {
    return render_html(blocks_);
}

const std::vector<BlockPtr>& MarkdownParser::blocks() const {
    return blocks_;
}
