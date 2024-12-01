cmake_minimum_required(VERSION 3.29)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_GENERATOR "MinGW Makefiles")

project(start_up)

set(CMAKE_CXX_STANDARD 17)

# Include FetchContent
include(FetchContent)

# Declare fmt
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
)

# Declare Catch2
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
)

# Make the libraries available
FetchContent_MakeAvailable(fmt Catch2)

############################################################
# Create a library
############################################################

add_library(monkey_library STATIC
        src/hello.cpp
        src/token/token.cpp
        src/ast/ast.cpp
        src/code/code.cpp
        src/object/object.cpp
        src/object/environment.cpp
        src/object/builtins.cpp
        src/lexer/lexer.cpp
        src/parser/parser.cpp
        src/parser/parser_tracing.cpp
        src/compiler/compiler.cpp
        src/compiler/symbol_table.cpp
)
add_library(monkey::library ALIAS monkey_library)

target_include_directories(monkey_library
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        PRIVATE
        ${PROJECT_SOURCE_DIR}/ast
        ${PROJECT_SOURCE_DIR}/code
        ${PROJECT_SOURCE_DIR}/compiler
        ${PROJECT_SOURCE_DIR}/lexer
        ${PROJECT_SOURCE_DIR}/parser
        ${PROJECT_SOURCE_DIR}/object
        ${PROJECT_SOURCE_DIR}/token
)

target_link_libraries(monkey_library PRIVATE fmt::fmt)

############################################################
# Create Main executable
############################################################

add_executable(start_up
        src/main.cpp
)

# Link libraries to main executable
target_link_libraries(start_up PRIVATE monkey::library)

############################################################
# Create Test executable
############################################################

add_executable(unit_tests
        test/unit_tests.cpp
        test/ast_tests.cpp
        test/code_tests.cpp
        test/object_tests.cpp
        test/lexer_tests.cpp
        test/parser_tests.cpp
        test/compiler_tests.cpp
        test/symbol_table_tests.cpp
)

# Link libraries to test executable
target_link_libraries(unit_tests PRIVATE
        monkey::library
        Catch2::Catch2WithMain
)
