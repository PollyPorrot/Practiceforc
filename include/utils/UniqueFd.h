#pragma once
#include <memory>
#include <unistd.h>

struct CloseDeleter{
    void operator()(int* p)const noexcept{
        if(!p)return ;
        if(*p>=0)::close(*p);
        delete p; // 释放动态分配的 int 内存
    }
};

using UniqueFd =std::unique_ptr<int,CloseDeleter>;

inline UniqueFd make_fd(int fd){
    if (fd < 0) {
        return nullptr; // 无效 fd 直接返回空指针
    }
    return UniqueFd(new int(fd)); 
}