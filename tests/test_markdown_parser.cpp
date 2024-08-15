#include "test_framework.h"

#include "markdown_parser.h"


int main() {
    MarkdownParser parser;
    CHECK(parser.to_html().empty());

    parser.set_text("# Hello Markdown");
    parser.parse();
    CHECK(parser.to_html().empty());

    MarkdownParser parser_with_text("some text");
    parser_with_text.parse();
    CHECK(parser_with_text.to_html().empty());

    std::printf("test_markdown_parser: all checks passed\n");
    return 0;
}
