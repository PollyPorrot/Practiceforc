#include "StringLite.h"
#include <iostream>

int main(){
    StringLite a("Hello");
    StringLite b=a;
    StringLite c;
    c=a;
    
    std::cout << a.c_str()<<"\n";
    std::cout<<b.c_str()<<"\n";
    std::cout<<c.c_str()<<"\n";
    return 0;
}
