#include <string>

#include "test_framework.h"
#include "markdown_parser.h"

// 验收标准：Markdown 文本经完整流水线后渲染出精确的 HTML。
// 约定：每个块级标签占一行（末尾带 '\n'）。

static std::string render(const std::string& md) {
    MarkdownParser parser(md);
    parser.parse();
    return parser.to_html();
}

static void test_heading_paragraph_hr() {
    CHECK_EQ(render("# Title"), std::string("<h1>Title</h1>\n"));
    CHECK_EQ(render("### Deep"), std::string("<h3>Deep</h3>\n"));
    CHECK_EQ(render("hello"), std::string("<p>hello</p>\n"));
    CHECK_EQ(render("---"), std::string("<hr />\n"));
}

static void test_escape_html() {
    // & 必须最先转义；< > " 都要变成实体
    CHECK_EQ(render("a < b & c > d"),
             std::string("<p>a &lt; b &amp; c &gt; d</p>\n"));
    // 文本里的标签不能变成真正的 HTML
    CHECK_EQ(render("<script>x</script>"),
             std::string("<p>&lt;script&gt;x&lt;/script&gt;</p>\n"));
}

static void test_emphasis_and_code() {
    CHECK_EQ(render("**b** and *i*"),
             std::string("<p><strong>b</strong> and <em>i</em></p>\n"));
    CHECK_EQ(render("a `x < y` b"),
             std::string("<p>a <code>x &lt; y</code> b</p>\n"));
    // 嵌套：粗体里套斜体
    CHECK_EQ(render("**a *b* c**"),
             std::string("<p><strong>a <em>b</em> c</strong></p>\n"));
}

static void test_link_and_image() {
    CHECK_EQ(render("[GitHub](https://github.com)"),
             std::string("<p><a href=\"https://github.com\">GitHub</a></p>\n"));
    // URL 中的 & 也要转义
    CHECK_EQ(render("[x](http://a?b=1&c=2)"),
             std::string("<p><a href=\"http://a?b=1&amp;c=2\">x</a></p>\n"));
    CHECK_EQ(render("![a cat](cat.png)"),
             std::string("<p><img src=\"cat.png\" alt=\"a cat\" /></p>\n"));
}

static void test_code_block() {
    CHECK_EQ(render("```c++\nint x = 1;\n```"),
             std::string(
                 "<pre><code class=\"language-c++\">int x = 1;</code></pre>\n"));
    // 无语言名时没有 class 属性；块内 < 要转义、换行保留
    CHECK_EQ(render("```\na < b\nc\n```"),
             std::string("<pre><code>a &lt; b\nc</code></pre>\n"));
}

static void test_lists() {
    CHECK_EQ(render("- a\n- b"),
             std::string("<ul>\n<li>a</li>\n<li>b</li>\n</ul>\n"));
    CHECK_EQ(render("1. one\n2. two"),
             std::string("<ol>\n<li>one</li>\n<li>two</li>\n</ol>\n"));
    // 列表项里的行内格式也要渲染
    CHECK_EQ(render("- **bold** item"),
             std::string("<ul>\n<li><strong>bold</strong> item</li>\n</ul>\n"));
}

static void test_blockquote() {
    CHECK_EQ(render("> quoted *text*"),
             std::string("<blockquote><p>quoted <em>text</em></p>"
                         "</blockquote>\n"));
}

static void test_full_document() {
    const std::string md =
        "# Doc\n"
        "\n"
        "Hello **world**\n"
        "\n"
        "- one\n"
        "- two\n"
        "\n"
        "---\n";
    const std::string expected =
        "<h1>Doc</h1>\n"
        "<p>Hello <strong>world</strong></p>\n"
        "<ul>\n"
        "<li>one</li>\n"
        "<li>two</li>\n"
        "</ul>\n"
        "<hr />\n";
    CHECK_EQ(render(md), expected);
}

int main() {
    test_heading_paragraph_hr();
    test_escape_html();
    test_emphasis_and_code();
    test_link_and_image();
    test_code_block();
    test_lists();
    test_blockquote();
    test_full_document();

    std::printf("test_renderer: all checks passed\n");
    return 0;
}
