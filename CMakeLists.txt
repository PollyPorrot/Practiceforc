cmake_minimum_required(VERSION 3.10)
project(cpp_bigproj_prep)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(dummy src/main.cpp)

add_executable(test_counter
    tests/test_counter.cpp
    src/Counter.cpp
)

add_executable(test_stringlite
    tests/test_stringlite.cpp
    src/StringLite.cpp
)