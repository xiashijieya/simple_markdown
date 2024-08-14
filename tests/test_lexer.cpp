#include <string>
#include <vector>

#include "test_framework.h"
#include "lexer.h"


static void test_split_lines_basic() {
    const std::vector<std::string> empty = split_lines("");
    CHECK(empty.empty());

    const std::vector<std::string> single = split_lines("hello");
    CHECK_EQ(single.size(), static_cast<size_t>(1));
    CHECK_EQ(single[0], std::string("hello"));

    const std::vector<std::string> two = split_lines("a\nb");
    CHECK_EQ(two.size(), static_cast<size_t>(2));
    CHECK_EQ(two[0], std::string("a"));
    CHECK_EQ(two[1], std::string("b"));
}

static void test_split_lines_endings() {
    // 末尾换行不产生额外空行
    const std::vector<std::string> trailing = split_lines("a\nb\n");
    CHECK_EQ(trailing.size(), static_cast<size_t>(2));

    // 中间空行必须保留
    const std::vector<std::string> blank = split_lines("a\n\nb");
    CHECK_EQ(blank.size(), static_cast<size_t>(3));
    CHECK_EQ(blank[1], std::string(""));

    // Windows CRLF：'\r' 要被剥掉
    const std::vector<std::string> crlf = split_lines("a\r\nb\r\n");
    CHECK_EQ(crlf.size(), static_cast<size_t>(2));
    CHECK_EQ(crlf[0], std::string("a"));
    CHECK_EQ(crlf[1], std::string("b"));
}

static void check_type(const std::string& line, LineType type) {
    const LineInfo info = classify_line(line);
    CHECK(info.type_ == type);
}

static void test_classify_blank_and_text() {
    check_type("", LineType::Blank);
    check_type("   \t ", LineType::Blank);
    check_type("just a paragraph", LineType::Text);

    const LineInfo text = classify_line("plain line");
    CHECK_EQ(text.content_, std::string("plain line"));
    CHECK_EQ(text.level_, 0);
}

static void test_classify_heading() {
    const LineInfo h1 = classify_line("# Title");
    CHECK(h1.type_ == LineType::Heading);
    CHECK_EQ(h1.level_, 1);
    CHECK_EQ(h1.content_, std::string("Title"));

    const LineInfo h3 = classify_line("### Deep Title");
    CHECK(h3.type_ == LineType::Heading);
    CHECK_EQ(h3.level_, 3);
    CHECK_EQ(h3.content_, std::string("Deep Title"));

    const LineInfo h6 = classify_line("###### Six");
    CHECK(h6.type_ == LineType::Heading);
    CHECK_EQ(h6.level_, 6);

    // '#' 后没有空格不是标题；7 个 '#' 也不是（最多 6 级）
    check_type("#nospace", LineType::Text);
    check_type("####### Seven", LineType::Text);
}

static void test_classify_code_fence() {
    const LineInfo fence = classify_line("```");
    CHECK(fence.type_ == LineType::CodeFence);
    CHECK_EQ(fence.content_, std::string(""));

    const LineInfo fence_lang = classify_line("```c++");
    CHECK(fence_lang.type_ == LineType::CodeFence);
    CHECK_EQ(fence_lang.content_, std::string("c++"));

    const LineInfo tilde = classify_line("~~~python");
    CHECK(tilde.type_ == LineType::CodeFence);
    CHECK_EQ(tilde.content_, std::string("python"));
}

static void test_classify_horizontal_rule() {
    check_type("---", LineType::HorizontalRule);
    check_type("***", LineType::HorizontalRule);
    check_type("___", LineType::HorizontalRule);
    check_type("* * *", LineType::HorizontalRule);
    check_type("  - - -  ", LineType::HorizontalRule);

    // 不足 3 个、或混入其他字符，都不是分割线
    check_type("--", LineType::Text);
    check_type("--*", LineType::Text);

    // "- - x" 不是分割线，但它合法命中无序列表规则（内容为 "- x"）
    const LineInfo nested_dash = classify_line("- - x");
    CHECK(nested_dash.type_ != LineType::HorizontalRule);
    CHECK(nested_dash.type_ == LineType::UnorderedList);
    CHECK_EQ(nested_dash.content_, std::string("- x"));
}

static void test_classify_blockquote() {
    const LineInfo q = classify_line("> quoted");
    CHECK(q.type_ == LineType::Blockquote);
    CHECK_EQ(q.content_, std::string("quoted"));

    const LineInfo q_nospace = classify_line(">quoted");
    CHECK(q_nospace.type_ == LineType::Blockquote);
    CHECK_EQ(q_nospace.content_, std::string("quoted"));
}

static void test_classify_lists() {
    const LineInfo ul_dash = classify_line("- item");
    CHECK(ul_dash.type_ == LineType::UnorderedList);
    CHECK_EQ(ul_dash.content_, std::string("item"));

    check_type("* star item", LineType::UnorderedList);
    check_type("+ plus item", LineType::UnorderedList);

    const LineInfo ol = classify_line("1. first");
    CHECK(ol.type_ == LineType::OrderedList);
    CHECK_EQ(ol.content_, std::string("first"));

    const LineInfo ol_big = classify_line("12. twelfth");
    CHECK(ol_big.type_ == LineType::OrderedList);
    CHECK_EQ(ol_big.content_, std::string("twelfth"));

    // marker 后没有空格不算列表
    check_type("-nope", LineType::Text);
    check_type("1.nope", LineType::Text);
}

static void test_priority_order() {
    // "---" 必须是分割线而不是无序列表项
    check_type("---", LineType::HorizontalRule);
    // "> - x" 首先是引用
    check_type("> - x", LineType::Blockquote);
}

static void test_tokenize_integration() {
    const std::string md = "# Title\n\n- a\n- b\n";
    const std::vector<LineInfo> lines = tokenize(md);
    CHECK_EQ(lines.size(), static_cast<size_t>(4));
    CHECK(lines[0].type_ == LineType::Heading);
    CHECK(lines[1].type_ == LineType::Blank);
    CHECK(lines[2].type_ == LineType::UnorderedList);
    CHECK(lines[3].type_ == LineType::UnorderedList);
}

int main() {
    test_split_lines_basic();
    test_split_lines_endings();
    test_classify_blank_and_text();
    test_classify_heading();
    test_classify_code_fence();
    test_classify_horizontal_rule();
    test_classify_blockquote();
    test_classify_lists();
    test_priority_order();
    test_tokenize_integration();

    std::printf("test_lexer: all checks passed\n");
    return 0;
}
