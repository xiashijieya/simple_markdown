#ifndef LINE_H
#define LINE_H

#include <string>

// 一行 Markdown 的语法类型。
// enum class 是 C++11 的“强类型枚举”：不会隐式转成整数，使用时必须带
// 限定名 LineType::Heading，能有效避免枚举值重名和误用。
enum class LineType {
    Unknown,
    Blank,           // 空行（没有任何内容，或只有空白字符）
    Heading,         // ATX 标题：# ~ ######
    UnorderedList,   // 无序列表项：- / * / + 后跟空格
    OrderedList,     // 有序列表项：数字 + . + 空格
    CodeFence,       // 代码块围栏行：``` 或 ~~~
    Blockquote,      // 引用块：> 开头
    HorizontalRule,  // 水平分割线：--- / *** / ___
    Text,            // 普通文本行（段落的一部分）
};

// 词法分析的结果单元：描述“一行是什么、内容是什么”。
// 注意：本阶段只做逐行识别，不处理跨行结构（如多行段落、列表嵌套、
// 围栏代码块的开始/结束配对），那些是阶段三“语法分析”的工作。
struct LineInfo {
    // 类内默认初始化（C++11）：即使不显式赋值，成员也有确定的初值。
    LineType type_;
    std::string content_;  // 去掉语法前缀后的文本；普通文本行则为整行
    int level_;        // 仅 Heading 使用：1~6；其他类型保持 0
    LineInfo():
    type_(LineType::Unknown), level_(0) {}
    LineInfo(LineType type, std::string content, int level) :
    type_(type), content_(content), level_(level) {}

};

#endif  // LINE_H
