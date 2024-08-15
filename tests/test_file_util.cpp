#include <cstdio>
#include <string>

#include "test_framework.h"
#include "file_util.h"

// 文件 IO 工具的往返测试与错误路径。
// 临时文件写在当前工作目录（build_debug/），.gitignore 已忽略。

static void test_read_nonexistent() {
    std::string out = "unchanged";
    // 打不开必须返回 false —— 与“空文件返回 true + 空串”区分开
    CHECK(!read_file("this_file_should_not_exist_xyz123.md", out));
}

static void test_write_and_read_roundtrip() {
    const std::string path = "test_file_util_tmp.txt";
    const std::string content = "# Hello\n\nline2 <b>&amp;</b>\n";

    CHECK(write_file(path, content));

    std::string got;
    CHECK(read_file(path, got));
    CHECK_EQ(got, content);

    std::remove(path.c_str());
}

static void test_empty_file_is_success() {
    const std::string path = "test_file_util_empty.tmp";
    CHECK(write_file(path, ""));

    std::string got = "sentinel";
    CHECK(read_file(path, got));
    CHECK(got.empty());  // 空文件：成功打开，内容为空

    std::remove(path.c_str());
}

static void test_overwrite_existing() {
    const std::string path = "test_file_util_over.tmp";
    CHECK(write_file(path, "first version, quite long"));
    CHECK(write_file(path, "short"));  // 覆盖：旧内容应被完全替换

    std::string got;
    CHECK(read_file(path, got));
    CHECK_EQ(got, std::string("short"));

    std::remove(path.c_str());
}

int main() {
    test_read_nonexistent();
    test_write_and_read_roundtrip();
    test_empty_file_is_success();
    test_overwrite_existing();

    std::printf("test_file_util: all checks passed\n");
    return 0;
}
