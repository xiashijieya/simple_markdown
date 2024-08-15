#include <string>
#include <vector>

#include "test_framework.h"
#include "ast.h"
#include "lexer.h"
#include "markdown_parser.h"
#include "parser.h"

// 阶段三验收标准：parse_blocks 把“带类型的行”组装成正确的 AST。
// 查看节点的具体内容时，需要用 dynamic_cast 把基类指针转成子类，
// 失败会返回 nullptr —— 这也是“多态 + 向下转型”的标准用法。

static void test_heading_and_hr() {
    const std::vector<LineInfo> lines = tokenize("# Title\n\n---\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(2));

    CHECK(doc[0]->type() == BlockType::Heading);
    const HeadingNode* h = dynamic_cast<const HeadingNode*>(doc[0].get());
    CHECK(h != 0);
    CHECK_EQ(h->level(), 1);
    CHECK_EQ(h->text(), std::string("Title"));

    CHECK(doc[1]->type() == BlockType::HorizontalRule);
}

static void test_paragraph_joins_lines() {
    // 两个连续 Text 行合并为一个段落，用空格连接
    const std::vector<LineInfo> lines = tokenize("hello\nworld\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const ParagraphNode* p = dynamic_cast<const ParagraphNode*>(doc[0].get());
    CHECK(p != 0);
    CHECK_EQ(p->text(), std::string("hello world"));
}

static void test_paragraph_separated_by_blank() {
    // 空行分隔成两个段落
    const std::vector<LineInfo> lines = tokenize("aaa\n\nbbb\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(2));
    CHECK(doc[0]->type() == BlockType::Paragraph);
    CHECK(doc[1]->type() == BlockType::Paragraph);
}

static void test_code_block_basic() {
    const std::string md = "```c++\nint x = 1;\n# not a heading\n```\n";
    const std::vector<LineInfo> lines = tokenize(md);
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const CodeBlockNode* code = dynamic_cast<const CodeBlockNode*>(doc[0].get());
    CHECK(code != 0);
    CHECK_EQ(code->language(), std::string("c++"));
    // 块内必须保留原文：# 没有被剥掉，两行用 '\n' 连接
    CHECK_EQ(code->content(), std::string("int x = 1;\n# not a heading"));
}

static void test_code_block_no_language_and_unclosed() {
    // 无语言名 + 没有结束围栏（未闭合代码块也要正常处理）
    const std::string md = "```\nraw text\n";
    const std::vector<LineInfo> lines = tokenize(md);
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const CodeBlockNode* code = dynamic_cast<const CodeBlockNode*>(doc[0].get());
    CHECK(code != 0);
    CHECK_EQ(code->language(), std::string(""));
    CHECK_EQ(code->content(), std::string("raw text"));
}

static void test_unordered_list() {
    const std::vector<LineInfo> lines = tokenize("- a\n- b\n- c\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const ListNode* list = dynamic_cast<const ListNode*>(doc[0].get());
    CHECK(list != 0);
    CHECK(!list->ordered());
    CHECK_EQ(list->items().size(), static_cast<size_t>(3));
    CHECK_EQ(list->items()[0], std::string("a"));
    CHECK_EQ(list->items()[2], std::string("c"));
}

static void test_ordered_list_and_mixed_types_split() {
    const std::vector<LineInfo> lines = tokenize("1. one\n2. two\n\n- x\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    // 有序列表 + 无序列表（被空行分开，且类型不同）应是两个节点
    CHECK_EQ(doc.size(), static_cast<size_t>(2));

    const ListNode* ol = dynamic_cast<const ListNode*>(doc[0].get());
    CHECK(ol != 0);
    CHECK(ol->ordered());
    CHECK_EQ(ol->items().size(), static_cast<size_t>(2));

    const ListNode* ul = dynamic_cast<const ListNode*>(doc[1].get());
    CHECK(ul != 0);
    CHECK(!ul->ordered());
    CHECK_EQ(ul->items().size(), static_cast<size_t>(1));
}

static void test_blockquote_example() {
    const std::vector<LineInfo> lines = tokenize("> line1\n> line2\n");
    const std::vector<BlockPtr> doc = parse_blocks(lines);

    CHECK_EQ(doc.size(), static_cast<size_t>(1));
    const BlockquoteNode* q = dynamic_cast<const BlockquoteNode*>(doc[0].get());
    CHECK(q != 0);
    CHECK_EQ(q->text(), std::string("line1\nline2"));
}

static void test_full_document_order() {
    const std::string md =
        "# H\n"
        "\n"
        "para one\n"
        "para two\n"
        "\n"
        "- a\n"
        "- b\n"
        "\n"
        "> quote\n"
        "\n"
        "```\n"
        "code\n"
        "```\n";
    const std::vector<BlockPtr> doc = parse_blocks(tokenize(md));

    CHECK_EQ(doc.size(), static_cast<size_t>(5));
    CHECK(doc[0]->type() == BlockType::Heading);
    CHECK(doc[1]->type() == BlockType::Paragraph);
    CHECK(doc[2]->type() == BlockType::UnorderedList);
    CHECK(doc[3]->type() == BlockType::Blockquote);
    CHECK(doc[4]->type() == BlockType::CodeBlock);
}

static void test_parser_class_fills_ast() {
    // 端到端：MarkdownParser.parse() 之后 blocks() 非空且类型正确
    MarkdownParser parser("# Hi\n\nbody\n");
    parser.parse();
    const std::vector<BlockPtr>& doc = parser.blocks();

    CHECK_EQ(doc.size(), static_cast<size_t>(2));
    CHECK(doc[0]->type() == BlockType::Heading);
    CHECK(doc[1]->type() == BlockType::Paragraph);
}

int main() {
    test_heading_and_hr();
    test_paragraph_joins_lines();
    test_paragraph_separated_by_blank();
    test_code_block_basic();
    test_code_block_no_language_and_unclosed();
    test_unordered_list();
    test_ordered_list_and_mixed_types_split();
    test_blockquote_example();
    test_full_document_order();
    test_parser_class_fills_ast();

    std::printf("test_parser: all checks passed\n");
    return 0;
}
