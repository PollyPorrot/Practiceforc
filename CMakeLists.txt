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

add_executable(test_repo
    tests/test_repo.cpp
    src/StudentRepo.cpp
)

add_executable(tcp_client
    net/tcp_client.cpp
)

add_executable(tcp_server
    net/tcp_server.cpp
)

#编译静态库名字叫msgcodec
add_library(msgcodec STATIC 
    src/net/MsgCodec.cpp
)

# 去哪找，不仅自己能用，链接自己的也能用
target_include_directories(msgcodec PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接到对应的可执行文件
target_link_libraries(tcp_server PRIVATE msgcodec)
target_link_libraries(tcp_client PRIVATE msgcodec)

# 如果想直接在外面编译不用cmake全部编译一遍的话
# g++ net/epoll_server.cpp build/libmsgcodec.a -I include -o epoll_server
# 把库直接放在前面，然后再跟上源在哪里
add_executable(epoll_server 
    net/epoll_server.cpp
)

target_link_libraries(epoll_server PRIVATE msgcodec)