#pragma once

template<typename T>
class MyUniquePtr{
public:
    explicit MyUniquePtr(T* ptr = nullptr) : ptr_(ptr) {}
    ~MyUniquePtr(){
        delete ptr_;
    }

    T* get() const {
        return ptr_;
    }

    // 这里为什么要用&，当然是不为什么，额，骗你的其实有原因的，，
    // 虽然但是我也没有找到一个很好的解释
    // 只会分析了倘若没有这个&那么是不是就是又新建了一个东西，使得两个对象共用一个资源这怎么行，所以加一个&，变成引用就好了，只是多了一个别名，并没有新建出新的变量
    T& operator*(){
        return *ptr_;
    }
    //这里为啥用*，因为你就是要一个指针啊老哥
    T* operator->(){
        return ptr_;
    }

    MyUniquePtr(const MyUniquePtr&)=delete;
    MyUniquePtr& operator=(const MyUniquePtr&)=delete;

    MyUniquePtr(const MyUniquePtr&& other) noexcept : ptr_(other.ptr_){
        other.ptr_ =nullptr;
    }
    MyUniquePtr& operator=(MyUniquePtr&& other)noexcept{
        if(this != &other){
            delete ptr_;
            ptr_ =other.ptr_;
            other.ptr_=nullptr;
        }
        return *this;
    }
private:
    T* ptr_;
};