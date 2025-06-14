#include"Timestamp.h"
#include<time.h>

TimeStamp TimeStamp::now(){
    return TimeStamp(time(NULL));
}

std::string TimeStamp::toString()const{
    char buffer[128]={0};
    struct tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buffer,128,"%04d-%02d-%02d %02d:%02d:%02d",
                        tm_time->tm_year+1900,
                        tm_time->tm_mon+1,
                        tm_time->tm_mday,
                        tm_time->tm_hour,
                        tm_time->tm_min,
                        tm_time->tm_sec);
    return buffer;
}
