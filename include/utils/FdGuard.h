#pragma once

#include <unistd.h>
class FdGuard{
public:
    FdGuard()=default;

    // explicit 防止隐形转换
    /*
        没有,以下可以编译
            FdGuard g=3;
        有的话，上面的就不能编译
            只能 FdGuard g(3);
    */
    explicit FdGuard(int fd) : fd_(fd) {};
    ~FdGuard(){reset();}

    // 不能拷贝，不然会两个对象持有同一个 fd，析构两次 close → 未定义行为/关错 fd
    FdGuard(const FdGuard& )=delete;
    FdGuard& operator=(const FdGuard&)=delete;

    // 保证不抛异常
    // STL（vector、map）在 move 时 只信任 noexcept
    // move 构造 / move 赋值，基本都要写 noexcept
    FdGuard(FdGuard&& other)noexcept : fd_(other.fd_){other.fd_=-1;}
    FdGuard& operator=(FdGuard&& other) noexcept {
        if(this!=&other){
            reset();
            fd_=other.fd_;
            other.fd_=-1;
        }
        return *this;
    }
    int get() const { return fd_; }
    int release() { int t = fd_; fd_ = -1; return t; }

    //close(fd);如果 fd<0  有问题
    void reset(int newfd = -1) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = newfd;
    }

private:
    int fd_ = -1;
};

