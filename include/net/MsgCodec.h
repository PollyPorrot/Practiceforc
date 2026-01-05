#pragma once
#include <string>

namespace net {
    bool recvAll(int fd, void* buf, size_t n);
    bool sendAll(int fd, const void* buf, size_t n);
    bool sendMsg(int fd, const std::string& msg);
    bool recvMsg(int fd, std::string& out);
}