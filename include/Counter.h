#pragma once

class Counter {
public:
    Counter();
    void inc();
    void add(int x);
    int value() const;

private:
    int value_;

};