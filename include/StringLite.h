#pragma once

#include <cstddef>

class StringLite{
public:
    StringLite();
    StringLite(const char * str);
    StringLite(const StringLite &other);
    StringLite& operator=(const StringLite &other);
    ~StringLite();

    const char* c_str() const;
    std::size_t size() const;

private:
    char* data_;
    std::size_t size_;
};