#include <string>
#include <vector>

#include "test_framework.h"
#include "inline_ast.h"
#include "inline_parser.h"
#include "markdown_parser.h"
#include "ast.h"

// 阶段四验收标准：parse_inlines 递归下降，产出正确的行内节点树。

static const TextNode* as_text(const InlineNode* node) {
    return dynamic_cast<const TextNode*>(node);
}

// 断言“恰好一个子节点，且是文本内容为 expected 的 TextNode”
static void check_sole_text(const std::vector<InlinePtr>& nodes,
                            const std::string& expected) {
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const TextNode* t = as_text(nodes[0].get());
    CHECK(t != 0);
    CHECK_EQ(t->text(), expected);
}

static void test_plain_and_empty() {
    check_sole_text(parse_inlines("hello"), std::string("hello"));
    CHECK(parse_inlines("").empty());
}

static void test_code_span() {
    const std::vector<InlinePtr> nodes = parse_inlines("`int x = 1;`");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const CodeNode* code = dynamic_cast<const CodeNode*>(nodes[0].get());
    CHECK(code != 0);
    CHECK_EQ(code->text(), std::string("int x = 1;"));

    // 代码内部的标记原样保留，不解析
    const std::vector<InlinePtr> raw = parse_inlines("`a * b ** c`");
    const CodeNode* raw_code = dynamic_cast<const CodeNode*>(raw[0].get());
    CHECK(raw_code != 0);
    CHECK_EQ(raw_code->text(), std::string("a * b ** c"));

    // 未闭合的反引号退化为字面量文本，且不产生 Code 节点
    const std::vector<InlinePtr> unclosed = parse_inlines("`abc");
    CHECK(dynamic_cast<const CodeNode*>(unclosed[0].get()) == 0);
}

static void test_bold() {
    const std::vector<InlinePtr> nodes = parse_inlines("**bold**");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const BoldNode* bold = dynamic_cast<const BoldNode*>(nodes[0].get());
    CHECK(bold != 0);
    check_sole_text(bold->children(), std::string("bold"));
}

static void test_italic() {
    const std::vector<InlinePtr> nodes = parse_inlines("*italic*");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const ItalicNode* it = dynamic_cast<const ItalicNode*>(nodes[0].get());
    CHECK(it != 0);
    check_sole_text(it->children(), std::string("italic"));
}

static void test_mixed_sequence() {
    // "a **b** c" -> Text, Bold, Text
    const std::vector<InlinePtr> nodes = parse_inlines("a **b** c");
    CHECK_EQ(nodes.size(), static_cast<size_t>(3));
    CHECK(as_text(nodes[0].get()) != 0);
    CHECK_EQ(as_text(nodes[0].get())->text(), std::string("a "));
    CHECK(dynamic_cast<const BoldNode*>(nodes[1].get()) != 0);
    CHECK_EQ(as_text(nodes[2].get())->text(), std::string(" c"));
}

static void test_nested_emphasis() {
    // **a *b* c** -> Bold{ Text("a "), Italic{Text("b")}, Text(" c") }
    const std::vector<InlinePtr> nodes = parse_inlines("**a *b* c**");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const BoldNode* bold = dynamic_cast<const BoldNode*>(nodes[0].get());
    CHECK(bold != 0);
    const std::vector<InlinePtr>& children = bold->children();
    CHECK_EQ(children.size(), static_cast<size_t>(3));
    CHECK_EQ(as_text(children[0].get())->text(), std::string("a "));
    const ItalicNode* it = dynamic_cast<const ItalicNode*>(children[1].get());
    CHECK(it != 0);
    check_sole_text(it->children(), std::string("b"));
    CHECK_EQ(as_text(children[2].get())->text(), std::string(" c"));
}

static void test_link() {
    const std::vector<InlinePtr> nodes =
        parse_inlines("[GitHub](https://github.com)");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const LinkNode* link = dynamic_cast<const LinkNode*>(nodes[0].get());
    CHECK(link != 0);
    CHECK_EQ(link->url(), std::string("https://github.com"));
    check_sole_text(link->children(), std::string("GitHub"));
}

static void test_link_with_formatted_text() {
    // 链接文字内部允许嵌套行内格式
    const std::vector<InlinePtr> nodes = parse_inlines("[**bold**](u)");
    const LinkNode* link = dynamic_cast<const LinkNode*>(nodes[0].get());
    CHECK(link != 0);
    CHECK_EQ(link->url(), std::string("u"));
    CHECK_EQ(link->children().size(), static_cast<size_t>(1));
    CHECK(dynamic_cast<const BoldNode*>(link->children()[0].get()) != 0);
}

static void test_image() {
    const std::vector<InlinePtr> nodes =
        parse_inlines("![a cat](cat.png)");
    CHECK_EQ(nodes.size(), static_cast<size_t>(1));
    const ImageNode* img = dynamic_cast<const ImageNode*>(nodes[0].get());
    CHECK(img != 0);
    CHECK_EQ(img->alt(), std::string("a cat"));
    CHECK_EQ(img->url(), std::string("cat.png"));
}

static void test_unclosed_emphasis_is_literal() {
    // 孤立的星号不是斜体，整段不应出现任何 Bold/Italic 节点
    const std::vector<InlinePtr> nodes = parse_inlines("a * b");
    for (size_t i = 0; i < nodes.size(); ++i) {
        CHECK(nodes[i]->type() != InlineType::Bold);
        CHECK(nodes[i]->type() != InlineType::Italic);
    }
}

static void test_end_to_end_in_block() {
    // 行内解析已接入块级流水线：段落节点里直接就是行内树
    MarkdownParser parser("x **y** z\n");
    parser.parse();
    const std::vector<BlockPtr>& doc = parser.blocks();
    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const ParagraphNode* p = dynamic_cast<const ParagraphNode*>(doc[0].get());
    CHECK(p != 0);
    CHECK_EQ(p->inlines().size(), static_cast<size_t>(3));
    CHECK(dynamic_cast<const BoldNode*>(p->inlines()[1].get()) != 0);
}

int main() {
    test_plain_and_empty();
    test_code_span();
    test_bold();
    test_italic();
    test_mixed_sequence();
    test_nested_emphasis();
    test_link();
    test_link_with_formatted_text();
    test_image();
    test_unclosed_emphasis_is_literal();
    test_end_to_end_in_block();

    std::printf("test_inline_parser: all checks passed\n");
    return 0;
}
