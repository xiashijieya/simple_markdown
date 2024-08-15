#ifndef INLINE_AST_H
#define INLINE_AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

// 行内节点的种类（与块级 BlockType 平行的第二套类层次）
enum class InlineType {
    Text,     // 普通文本
    Bold,     // **粗体**
    Italic,   // *斜体*
    Code,     // `行内代码`（内部原样保存，不再解析）
    Link,     // [文本](url)
    Image,    // ![替代文本](url)
};

// 行内 AST 节点抽象基类。设计与 BlockNode 完全对应：
// 纯虚 type() + 虚析构函数，子类用 override 重写。
class InlineNode {
public:
    virtual ~InlineNode() {}
    virtual InlineType type() const = 0;
};

// 普通文本（未被任何标记包围的字符）
class TextNode : public InlineNode {
public:
    explicit TextNode(std::string text) : text_(std::move(text)) {}

    InlineType type() const override { return InlineType::Text; }
    const std::string& text() const { return text_; }

private:
    std::string text_;
};

// 粗体/斜体：内部可以再嵌套任意行内节点
// （例如 **粗体里有 `代码` 和 *斜体***），所以存子节点序列
class BoldNode : public InlineNode {
public:
    explicit BoldNode(std::vector<std::unique_ptr<InlineNode>> children)
        : children_(std::move(children)) {}

    InlineType type() const override { return InlineType::Bold; }
    const std::vector<std::unique_ptr<InlineNode>>& children() const {
        return children_;
    }

private:
    std::vector<std::unique_ptr<InlineNode>> children_;
};

class ItalicNode : public InlineNode {
public:
    explicit ItalicNode(std::vector<std::unique_ptr<InlineNode>> children)
        : children_(std::move(children)) {}

    InlineType type() const override { return InlineType::Italic; }
    const std::vector<std::unique_ptr<InlineNode>>& children() const {
        return children_;
    }

private:
    std::vector<std::unique_ptr<InlineNode>> children_;
};

// 行内代码：内容原样保存（不转义、不嵌套解析）
class CodeNode : public InlineNode {
public:
    explicit CodeNode(std::string text) : text_(std::move(text)) {}

    InlineType type() const override { return InlineType::Code; }
    const std::string& text() const { return text_; }

private:
    std::string text_;
};

// 链接：方括号内的文本本身也可以有行内格式，故为子节点序列
class LinkNode : public InlineNode {
public:
    LinkNode(std::vector<std::unique_ptr<InlineNode>> children,
             std::string url)
        : children_(std::move(children)), url_(std::move(url)) {}

    InlineType type() const override { return InlineType::Link; }
    const std::vector<std::unique_ptr<InlineNode>>& children() const {
        return children_;
    }
    const std::string& url() const { return url_; }

private:
    std::vector<std::unique_ptr<InlineNode>> children_;
    std::string url_;
};

// 图片：替代文本按纯文本保存
class ImageNode : public InlineNode {
public:
    ImageNode(std::string alt, std::string url)
        : alt_(std::move(alt)), url_(std::move(url)) {}

    InlineType type() const override { return InlineType::Image; }
    const std::string& alt() const { return alt_; }
    const std::string& url() const { return url_; }

private:
    std::string alt_;
    std::string url_;
};

using InlinePtr = std::unique_ptr<InlineNode>;

#endif  // INLINE_AST_H
