# SimpleMarkdown
这是一个用 C++11 实现的命令行 Markdown 转 HTML 工具。是一个小练习。
本项目采用「词法分析 →块级语法树 → 行内语法树 → HTML 渲染」的分层流水线。

## 功能特性

- 命令行转换：支持文件输入与标准输入，输出到文件或标准输出
- 导出的 HTML 为自包含完整文档，主题 CSS 直接内嵌，可单独分发
- 内置浅色主题（`theme/default.css`），也支持指定自定义 CSS
- 对未闭合标记做容错降级（按普通文本处理），不会中断解析

### 支持的 Markdown 语法
目前支持的 Markdown 语法有限，比如下面这个地方要用到的表格目前就没支持…… 也没做代码高亮（或许可以用highlight.js来偷懒实现）。

块级元素：

| 语法 | 说明 |
| --- | --- |
| `#` ~ `######` | ATX 标题（h1 ~ h6） |
| 普通文本行 | 连续行合并为段落 |
| ```` ``` ```` / `~~~` | 围栏代码块，支持语言标注（未闭合也能正常结束） |
| `>` | 引用块 |
| `-` / `*` / `+` | 无序列表 |
| `1.` | 有序列表 |
| `---` / `***` / `___` | 水平分割线 |

行内元素（支持相互嵌套）：

| 语法 | 输出 |
| --- | --- |
| `**粗体**` | `<strong>` |
| `*斜体*` | `<em>` |
| `` `代码` `` | `<code>` |
| `[文字](url)` | `<a>` 链接 |
| `![alt](url)` | `<img>` 图片 |

## 构建方法

环境要求：CMake 3.12 及以上、MinGW（g++）。

在项目根目录（Git Bash）执行配置脚本：

```bash
./build.sh debug      # 配置到 build_debug/   （Debug）
./build.sh release    # 配置到 build_release/ （RelWithDebInfo）
```

`build.sh` 只负责配置，编译进入对应构建目录执行：

```bash
cd build_debug
make                  # 产物：simple_markdown.exe
```

本项目以 `-Wall -Wextra -Wpedantic` 编译，保持零警告。

## 使用方法

```
simple_markdown [输入文件] [-o 输出文件] [-t 主题文件]
```

- 省略输入文件时从标准输入读取
- 省略 `-o` 时，HTML 片段输出到标准输出（不含 `<html>` 外壳，便于管道拼接）
- 使用 `-o` 时导出完整 HTML 文档，并把主题 CSS 内嵌到 `<style>` 中；
  文档标题取输出文件名
- 省略 `-t` 时使用默认主题 `theme/default.css`（相对于运行时的工作目录），
  分发时将 `theme/` 目录与可执行文件放在同一目录即可
- 主题文件读取失败时会输出警告，但仍正常导出不含样式的 HTML

示例：

```bash
# 渲染一下现在的这个README.md
./build_debug/simple_markdown.exe README.md -o README.html

# 文件转换，结果打印到标准输出
./build_debug/simple_markdown.exe doc.md

# 导出带样式的完整 HTML 文档
./build_debug/simple_markdown.exe doc.md -o doc.html

# 使用自定义主题
./build_debug/simple_markdown.exe doc.md -o doc.html -t my.css

# 从管道读取
cat doc.md | ./build_debug/simple_markdown.exe
```

## 测试

`tests/` 目录下的每个 `test_*.cpp` 都是独立单元测试，CMake 自动发现并
生成同名构建目标：

```bash
cd build_debug
make test_renderer        # 编译单个测试
./test_renderer.exe       # 运行
ctest                     # 一次性运行全部测试
```

当前共 6 个测试套件，覆盖文件读写、词法分析、块级解析、行内解析、
渲染与端到端流程。

## 目录结构

```
SimpleMarkdown/
├── CMakeLists.txt            # 构建脚本
├── build.sh                  # 构建配置脚本
├── theme/
│   └── default.css           # 内置主题（导出时内嵌进 HTML）
├── src/
│   ├── main.cpp              # 入口：argv 解析、读写文件、主题加载
│   ├── markdown_parser.h/.cpp# MarkdownParser：总装整条流水线
│   ├── line.h                # LineType 枚举与 LineInfo（词法单元）
│   ├── lexer.h/.cpp          # 词法分析：按行切分与逐行分类
│   ├── ast.h                 # 块级 AST 节点类层次
│   ├── parser.h/.cpp         # 块级语法分析：行序列 → 块级 AST
│   ├── inline_ast.h          # 行内 AST 节点类层次
│   ├── inline_parser.h/.cpp  # 行内语法分析：递归下降
│   ├── renderer.h/.cpp       # HTML 渲染：遍历 AST 输出
│   └── file_util.h/.cpp      # 文件读写
└── tests/
    ├── test_framework.h      # 极简测试框架（CHECK / CHECK_EQ）
    └── test_*.cpp            # 各阶段单元测试
```

## 处理流程

```
原始文本
  │  lexer          按行切分、逐行识别类型（无状态）
  ▼
行序列 vector<LineInfo>
  │  parser         组装跨行结构（段落合并、代码块配对等）
  ▼
块级 AST vector<BlockPtr>
  │  inline_parser  递归下降解析粗体/斜体/代码/链接/图片及嵌套
  ▼
块级 AST（含行内节点树）
  │  renderer       递归遍历节点、转义特殊字符、输出 HTML
  ▼
HTML 片段 ── 导出文件时包装文档外壳并内嵌 CSS ──▶ 完整 HTML 文件
```
