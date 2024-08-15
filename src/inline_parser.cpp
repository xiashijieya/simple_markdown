#include "inline_parser.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "inline_ast.h"

namespace {

bool starts_with(const std::string& s, size_t pos,
                 const std::string& marker) {
    return pos + marker.size() <= s.size() &&
           s.compare(pos, marker.size(), marker) == 0;
}

// ---------------------------------------------------------------------------
// 普通文本片段
//
// 从 s[pos] 开始，逐个字符收集“普通文本”，直到遇到下列任一情况停止
// （停止时【不要消费】触发字符，把 pos 停在它上面，交回主循环处理）：
//   1. 到达字符串末尾；
//   2. 遇到结束标记 stop（用 starts_with 判断；顶层调用时 stop 为 ""，
//      starts_with 对空 marker 永远为真，所以 stop 为空时要跳过本项判断）；
//   3. 遇到特殊字符 '`'（行内代码）或 '*'（粗体/斜体）；
//   4. 遇到 '['（链接）；
//   5. 遇到 '!' 且它后面紧跟 '['（图片）。
//
// 把收集到的字符构造为 TextNode 返回（调用方保证进入时至少有一个
// 普通字符）。
// ---------------------------------------------------------------------------
InlinePtr parse_text_run(const std::string& s, size_t& pos,
                         const std::string& stop) {
    const size_t begin = pos;
    while (pos < s.size()) {
        if ((!stop.empty() && starts_with(s, pos, stop))
            || s[pos] == '`'
            || s[pos] == '*'
            || s[pos] == '['
            || (pos + 1 < s.size() && s[pos] == '!' && s[pos + 1] == '[')) {
            break;
        }
        ++pos;
    }
    return InlinePtr(new TextNode(s.substr(begin, pos - begin)));
}

// ---------------------------------------------------------------------------
// 行内代码 `code`
//
// 进入时 s[pos] == '`'。
//   1. 用 s.find('`', pos + 1) 寻找闭合反引号；
//   2. 找不到：说明代码未闭合，按字面量处理 —— 返回 TextNode("`")，
//      且 pos 只前进 1（让主循环继续处理后续字符，避免死循环）；
//   3. 找到（位置记为 end）：内容为 s.substr(pos + 1, end - pos - 1)
//      （原样保存，哪怕里面有 * 或 # 也不解析），pos 推进到 end + 1，
//      返回 CodeNode。
// ---------------------------------------------------------------------------
InlinePtr parse_code_span(const std::string& s, size_t& pos) {
    const size_t end = s.find('`', pos + 1);
    if (end == std::string::npos) {
        ++pos;
        return InlinePtr(new TextNode("`"));
    }
    const std::string code = s.substr(pos + 1, end - pos - 1);
    pos = end + 1;
    return InlinePtr(new CodeNode(code));
}

// ---------------------------------------------------------------------------
// 链接 [text](url) 与图片 ![alt](url)
//
// is_image == true 时进入位置 s[pos] == '!'；否则 s[pos] == '['。
// 解析步骤：
//   1. 图片：先 ++pos 跳过 '!'，此时 s[pos] 应为 '['；
//   2. ++pos 跳过 '['；
//   3. 取方括号内容：
//      - 图片：用 s.find(']', pos) 找 ']'，取其中的纯文本作为 alt，
//        pos 推进到 ']' 之后；
//      - 链接：方括号内允许嵌套行内格式，请【递归】调用
//            parse_sequence(s, pos, "]")
//        得到子节点（它会在消费 ']' 后返回）；
//   4. 此时 s[pos] 应为 '('，再用 s.find(')', pos + 1) 找 ')'，
//      中间的子串即 url，pos 推进到 ')' 之后；
//   5. 分别构造 ImageNode(alt, url) 或 LinkNode(children, url) 返回。
//
// 容错：任何一步找不到期望的标记（']'、'('、')' 缺失），就退化为
// 字面量文本 —— 图片返回 TextNode("!")、链接返回 TextNode("[")。
// 注意此时 pos 已经越过首字符（'!' 或 '['），保持不变即可，
// 主循环会把剩下的 '[' 等字符继续当普通文本处理（不会死循环）。
//
// ---------------------------------------------------------------------------
std::vector<InlinePtr> parse_sequence(const std::string& s, size_t& pos,
                                      const std::string& stop);

InlinePtr parse_link(const std::string& s, size_t& pos, bool is_image) {
    std::string alt;
    // 图片：先跳过 '!'
    if (is_image) {
        if (pos >= s.size() || s[pos] != '!') {
            return InlinePtr(new TextNode("!"));
        }
        ++pos;
    }

    // 此时应遇到 '['
    if (pos >= s.size() || s[pos] != '[') {
        // 退化：把已经跳过的字符作为字面量返回
        if (is_image) {
            return InlinePtr(new TextNode("!"));
        }
        return InlinePtr(new TextNode("["));
    }
    ++pos;  // 跳过 '['

    if (is_image) {
        // 图片：alt 为纯文本，找最近的 ']'
        size_t end = s.find(']', pos);
        if (end == std::string::npos) {
            return InlinePtr(new TextNode("!"));
        }
        alt = s.substr(pos, end - pos);
        pos = end + 1;  // 跳过 ']'
    } else {
        // 链接：方括号内允许嵌套行内格式，递归解析
        // parse_sequence 会在消费到 ']' 后返回
        auto children = parse_sequence(s, pos, "]");
        if (children.empty()) {
            return InlinePtr(new TextNode("["));
        }
        std::vector<InlinePtr> link_text = std::move(children);

        // 接下来解析 url：期望 '('
        if (pos >= s.size() || s[pos] != '(') {
            return InlinePtr(new TextNode("["));
        }
        ++pos;  // 跳过 '('

        size_t url_end = s.find(')', pos);
        if (url_end == std::string::npos) {
            return InlinePtr(new TextNode("["));
        }
        std::string url = s.substr(pos, url_end - pos);
        pos = url_end + 1;  // 跳过 ')'

        return InlinePtr(new LinkNode(std::move(link_text), url));
    }

    // 图片分支：解析 url
    if (pos >= s.size() || s[pos] != '(') {
        return InlinePtr(new TextNode("!"));
    }
    ++pos;  // 跳过 '('

    size_t url_end = s.find(')', pos);
    if (url_end == std::string::npos) {
        return InlinePtr(new TextNode("!"));
    }
    std::string url = s.substr(pos, url_end - pos);
    pos = url_end + 1;  // 跳过 ')'

    return InlinePtr(new ImageNode(std::move(alt), url));
}

// ---------------------------------------------------------------------------
// 递归下降的核心。解析一段行内内容，直到遇到结束标记 stop
// （如 "**"、"*"、"]"）或字符串末尾；遇到 stop 时连它一起消费。
//
// “开标记 -> 递归 -> 闭标记”的递归下降
// ---------------------------------------------------------------------------
std::vector<InlinePtr> parse_sequence(const std::string& s, size_t& pos,
                                      const std::string& stop) {
    std::vector<InlinePtr> nodes;

    // 当前是否正停在结束标记上。特例：stop 为 "*" 时，"**" 必须优先
    // 交给粗体分支，不能在这里当作斜体的结束。
    // 用 lambda 每轮循环重新判断（pos 在不断推进，不能只算一次）。
    auto at_stop = [&]() {
        return !stop.empty() && starts_with(s, pos, stop) &&
               !(stop == "*" && starts_with(s, pos, "**"));
    };

    while (pos < s.size() && !at_stop()) {
        if (s[pos] == '`') {
            nodes.push_back(parse_code_span(s, pos));
        } else if (starts_with(s, pos, "**")) {
            // 粗体：先确认后面存在闭合的 "**"，否则按字面量处理
            const size_t close = s.find("**", pos + 2);
            if (close == std::string::npos) {
                nodes.push_back(InlinePtr(new TextNode("**")));
                pos += 2;
            } else {
                pos += 2;  // 消费开标记
                std::vector<InlinePtr> children =
                    parse_sequence(s, pos, "**");  // 递归，闭标记由它消费
                nodes.push_back(
                    InlinePtr(new BoldNode(std::move(children))));
            }
        } else if (s[pos] == '*') {
            // 斜体：寻找一个“单星号”闭合点（扫描时要成对跳过 "**"，
            // 否则粗体的两颗星会被误当成斜体闭合）
            size_t close = std::string::npos;
            size_t i = pos + 1;
            while (i < s.size()) {
                if (starts_with(s, i, "**")) {
                    i += 2;
                } else if (s[i] == '*') {
                    close = i;
                    break;
                } else {
                    ++i;
                }
            }
            if (close == std::string::npos) {
                nodes.push_back(InlinePtr(new TextNode("*")));
                ++pos;
            } else {
                ++pos;  // 消费开标记 '*'
                std::vector<InlinePtr> children =
                    parse_sequence(s, pos, "*");
                nodes.push_back(
                    InlinePtr(new ItalicNode(std::move(children))));
            }
        } else if (s[pos] == '!' && pos + 1 < s.size() &&
                   s[pos + 1] == '[') {
            nodes.push_back(parse_link(s, pos, true));
        } else if (s[pos] == '[') {
            nodes.push_back(parse_link(s, pos, false));
        } else {
            nodes.push_back(parse_text_run(s, pos, stop));
        }
    }

    // 消费结束标记（若到达字符串末尾则没有标记）
    if (!stop.empty() && starts_with(s, pos, stop)) {
        pos += stop.size();
    }
    return nodes;
}

}  // namespace

std::vector<InlinePtr> parse_inlines(const std::string& text) {
    size_t pos = 0;
    // 顶层调用没有结束标记，stop 传 ""
    return parse_sequence(text, pos, "");
}
