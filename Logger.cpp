#include"Logger.h"
#include<iostream>
#include"Timestamp.h"
void Logger::log(std::string& msg){
    switch (logLevel_)
    {
        case LogLevel::INFO:
            std::cout << "[INFO]: ";
            break;
        case LogLevel::DEBUG:   
            std::cout << "[DEBUG]: " ;
            break;
        case LogLevel::WARNING:
            std::cout << "[WARNING]: " ;
            break;
        case LogLevel::ERROR:
            std::cout << "[ERROR]: " ;
            break;
        case LogLevel::FATAL:
            std::cout << "[FATAL]: " ;
            break;
        default:
            std::cout << "[UNKNOWN]: ";
            break;      
    }

    std::cout<<"["<< TimeStamp::now().toString() <<"]"<< msg << std::endl;
}