#include "StringLite.h"
#include <cstddef>
#include<cstring>

StringLite::StringLite() : data_(nullptr),size_(0){}

StringLite::StringLite(const char * str){
    if(str){
        size_ = std::strlen(str);
        data_=new char[size_+1];
        std::strcpy(data_,str);
    }else{
        data_=nullptr;
        size_=0;
    }
}

StringLite::StringLite(const StringLite& other){
    size_=other.size_;
    if(other.data_){
        data_=new char[size_+1];
        std::strcpy(data_,other.data_);
    }else{
        data_=nullptr;
    }
}

StringLite& StringLite::operator=(const StringLite& other){
    if(this == &other){
        return *this;
    }

    delete[] data_;
    size_=other.size_;
    if(other.data_){
        data_ = new char[size_ + 1];
        std::strcpy(data_, other.data_);
    } else {
        data_ = nullptr;
    }

    return *this;
}

StringLite::~StringLite(){
    delete[] data_;
}

const char* StringLite::c_str() const{
    return data_ ? data_ : nullptr;
}

std::size_t StringLite::size() const{
    return size_;
}