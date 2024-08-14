#!/usr/bin/env bash
#
# SimpleMarkdown 构建配置脚本
#
# 用法（在 Git Bash 中，于项目根目录执行）：
#   ./build.sh debug     # 配置到 build_debug/   （Debug）
#   ./build.sh release   # 配置到 build_release/ （RelWithDebInfo）
#
# 本脚本只负责“配置”，编译请进入对应目录后用 make 完成：
#   cd build_debug
#   make                 # 编译主程序
#   make <testname>      # 编译某个单元测试（如 make test_markdown_parser）
#
set -euo pipefail

# 无论从哪里调用，都切换到脚本所在目录（项目根目录）
cd "$(dirname "$0")"

usage() {
    echo "用法: ./build.sh <debug|release>"
    echo "  debug   -> build_debug/   (CMAKE_BUILD_TYPE=Debug)"
    echo "  release -> build_release/ (CMAKE_BUILD_TYPE=RelWithDebInfo)"
    exit 1
}

[ "$#" -eq 1 ] || usage

case "$1" in
    debug)
        build_dir="build_debug"
        build_type="Debug"
        ;;
    release)
        build_dir="build_release"
        build_type="RelWithDebInfo"
        ;;
    *)
        usage
        ;;
esac

cmake -S . -B "${build_dir}" \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE="${build_type}"

cat <<EOF

配置完成：${build_dir}/ （${build_type}）
接下来：
  cd ${build_dir}
  make                    # 编译主程序（产物：simple_markdown.exe）
  make <testname>         # 编译单元测试，例如 make test_markdown_parser
  ctest                   # （可选）运行全部单元测试
EOF
