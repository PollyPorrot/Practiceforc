#include "Counter.h"

Counter::Counter() : value_(0){}

void Counter::inc(){
    ++value_;
}
void Counter::add(int x){
    value_+=x;
}
int Counter::value() const{
    return value_;
}