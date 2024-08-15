cmake_minimum_required(VERSION 3.12)
project(SimpleMarkdown CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(markdown_core STATIC
    src/markdown_parser.cpp
    src/lexer.cpp
    src/parser.cpp
    src/inline_parser.cpp
)
target_include_directories(markdown_core PUBLIC src)

add_executable(simple_markdown src/main.cpp)
target_link_libraries(simple_markdown PRIVATE markdown_core)

enable_testing()

file(GLOB MARKDOWN_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp"
)

foreach(test_src IN LISTS MARKDOWN_TEST_SOURCES)
    get_filename_component(test_name "${test_src}" NAME_WE)
    add_executable("${test_name}" "${test_src}")
    target_link_libraries("${test_name}" PRIVATE markdown_core)
    target_include_directories("${test_name}" PRIVATE tests)
    add_test(NAME "${test_name}" COMMAND "${test_name}")
endforeach()
