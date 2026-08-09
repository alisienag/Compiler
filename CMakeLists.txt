cmake_minimum_required(VERSION 3.20)
project(Compiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(Compiler src/main.cpp src/lexer.cpp src/parser.cpp src/stringtable.cpp src/cgen.cpp src/scopecheck.cpp src/typecheck.cpp)
target_include_directories(Compiler PRIVATE include)
target_compile_options(Compiler PRIVATE -Wall -Wextra -pedantic)
