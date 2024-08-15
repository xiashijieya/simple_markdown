#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "inline_ast.h"

// 块级节点的种类
enum class BlockType {
    Heading,
    Paragraph,
    CodeBlock,
    UnorderedList,
    OrderedList,
    Blockquote,
    HorizontalRule,
};

class BlockNode {
public:
    virtual ~BlockNode() {}
    virtual BlockType type() const = 0;
};

// 标题：# ~ ######
class HeadingNode : public BlockNode {
public:
    HeadingNode(int level, std::vector<InlinePtr> inlines)
        : level_(level), inlines_(std::move(inlines)) {}

    BlockType type() const override { return BlockType::Heading; }

    int level() const { return level_; }
    const std::vector<InlinePtr>& inlines() const { return inlines_; }

private:
    int level_;
    std::vector<InlinePtr> inlines_;
};

// 段落：连续普通文本行合并后，内容展开为行内节点树
class ParagraphNode : public BlockNode {
public:
    explicit ParagraphNode(std::vector<InlinePtr> inlines)
        : inlines_(std::move(inlines)) {}

    BlockType type() const override { return BlockType::Paragraph; }

    const std::vector<InlinePtr>& inlines() const { return inlines_; }

private:
    std::vector<InlinePtr> inlines_;
};

// 围栏代码块：语言名可为空；content_ 是块内全部原始文本，行之间用 '\n'。
// 代码块内容不做行内解析（** 和 [ 都按字面量显示）。
class CodeBlockNode : public BlockNode {
public:
    CodeBlockNode(std::string language, std::string content)
        : language_(std::move(language)), content_(std::move(content)) {}

    BlockType type() const override { return BlockType::CodeBlock; }

    const std::string& language() const { return language_; }
    const std::string& content() const { return content_; }

private:
    std::string language_;
    std::string content_;
};

// 列表：有序 / 无序；每个列表项本身是一棵行内节点树
class ListNode : public BlockNode {
public:
    ListNode(bool ordered, std::vector<std::vector<InlinePtr>> items)
        : ordered_(ordered), items_(std::move(items)) {}

    BlockType type() const override {
        return ordered_ ? BlockType::OrderedList
                        : BlockType::UnorderedList;
    }

    bool ordered() const { return ordered_; }
    const std::vector<std::vector<InlinePtr>>& items() const {
        return items_;
    }

private:
    bool ordered_;
    std::vector<std::vector<InlinePtr>> items_;
};

// 引用块：各行文本用 '\n' 连接后统一做行内解析
class BlockquoteNode : public BlockNode {
public:
    explicit BlockquoteNode(std::vector<InlinePtr> inlines)
        : inlines_(std::move(inlines)) {}

    BlockType type() const override { return BlockType::Blockquote; }

    const std::vector<InlinePtr>& inlines() const { return inlines_; }

private:
    std::vector<InlinePtr> inlines_;
};

// 水平分割线：--- / *** / ___
class HorizontalRuleNode : public BlockNode {
public:
    BlockType type() const override { return BlockType::HorizontalRule; }
};

// 节点所有权类型：文档持有节点的唯一所有权。
using BlockPtr = std::unique_ptr<BlockNode>;

#endif  // AST_H
