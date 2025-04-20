#pragma once
#include<iostream>
#include<string>
#include <sys/time.h>
class TimeStamp{
public:
    TimeStamp(): microSecondsSinceEpoch_(0) {};
    explicit TimeStamp(int64_t microSecondsSinceEpoch): 
        microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

    static TimeStamp now();
    std::string toString()const;
private:
    int64_t microSecondsSinceEpoch_;
};