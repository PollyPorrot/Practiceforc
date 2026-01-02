#include <iostream>
#include "Counter.h"

int main(){
    Counter c;
    c.inc();
    c.add(5);

    if(c.value() !=6){
        std::cout<<"Test failed\n";
        return 1;
    }

    std::cout<<"Test passed value ="<<c.value()<<"\n";
    return 0;
}