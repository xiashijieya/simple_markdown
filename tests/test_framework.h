#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

// 极简单元测试框架（仅用于本项目学习，不引入任何第三方依赖）。
//
// 用法：在 tests/test_xxx.cpp 中写一个 main()，用下面的宏做检查：
//
//   #include "test_framework.h"
//   int main() {
//       CHECK(1 + 1 == 2);
//       CHECK_EQ(std::string("ab"), std::string("ab"));
//       std::printf("test_xxx: all checks passed\n");
//       return 0;
//   }
//
// 检查失败时会打印失败位置并以非零退出码结束（ctest 据此判定失败）。

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

// 条件必须为真
#define CHECK(cond)                                                           \
    do {                                                                      \
        if (!(cond)) {                                                        \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,     \
                         __FILE__, __LINE__);                                 \
            std::exit(1);                                                     \
        }                                                                     \
    } while (0)

namespace test_framework {

// 借助 operator<< 把任意可打印值转成字符串，用于失败信息
template <typename T>
std::string to_string(const T& value) {
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

}  // namespace test_framework

// 两个值必须相等（类型需要支持 ==，且能用 << 打印）
#define CHECK_EQ(actual, expected)                                            \
    do {                                                                      \
        const auto check_eq_actual_ = (actual);                               \
        const auto check_eq_expected_ = (expected);                           \
        if (!(check_eq_actual_ == check_eq_expected_)) {                      \
            std::fprintf(stderr,                                              \
                         "CHECK_EQ failed: %s == %s\n  actual:   \"%s\"\n"    \
                         "  expected: \"%s\"\n  at %s:%d\n",                  \
                         #actual, #expected,                                  \
                         test_framework::to_string(check_eq_actual_).c_str(), \
                         test_framework::to_string(check_eq_expected_)        \
                             .c_str(),                                        \
                         __FILE__, __LINE__);                                 \
            std::exit(1);                                                     \
        }                                                                     \
    } while (0)

#endif  // TEST_FRAMEWORK_H
