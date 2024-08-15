#include <string>

#include "test_framework.h"

#include "markdown_parser.h"


int main() {
    MarkdownParser parser;
    // 尚未 parse：没有块，HTML 为空串
    CHECK(parser.to_html().empty());

    parser.set_text("# Hello Markdown");
    parser.parse();
    CHECK_EQ(parser.to_html(),
             std::string("<h1>Hello Markdown</h1>\n"));

    MarkdownParser parser_with_text("some text");
    parser_with_text.parse();
    CHECK_EQ(parser_with_text.to_html(),
             std::string("<p>some text</p>\n"));

    std::printf("test_markdown_parser: all checks passed\n");
    return 0;
}
