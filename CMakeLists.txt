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
        "src/hello.cpp"
)
add_library(monkey::library ALIAS monkey_library)

target_include_directories(monkey_library
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
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
)

# Link libraries to test executable
target_link_libraries(unit_tests PRIVATE 
    monkey::library
    Catch2::Catch2WithMain
)
