cmake_minimum_required(VERSION 3.10)
project(cpp_bigproj_prep)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 接口库，提供一个链接，不产生编译的内容
add_library(project_headers INTERFACE)
target_include_directories(project_headers INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # 配置头文件根目录
)

add_executable(dummy src/main.cpp)
# （可选）如果 dummy 也需要头文件，添加依赖
target_link_libraries(dummy PRIVATE project_headers)

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

# 修正1：用 target_link_libraries 依赖接口库，继承头文件路径（替换 target_link_directories）
target_link_libraries(test_counter PRIVATE project_headers)
target_link_libraries(test_stringlite PRIVATE project_headers)
target_link_libraries(test_repo PRIVATE project_headers)

add_executable(tcp_client
    net/tcp_client.cpp
)

add_executable(tcp_server
    net/tcp_server.cpp
)

# 编译静态库 msgcodec
add_library(msgcodec STATIC 
    src/net/MsgCodec.cpp
)

# 修正2：用 target_link_libraries 让 msgcodec 依赖接口库（PUBLIC 保证传递性）
target_link_libraries(msgcodec PUBLIC project_headers)

# 链接到对应的可执行文件
target_link_libraries(tcp_server PRIVATE msgcodec)
target_link_libraries(tcp_client PRIVATE msgcodec)

add_executable(epoll_server 
    net/epoll_server.cpp
)

target_link_libraries(epoll_server PRIVATE msgcodec)