#pragma once
#include"nocopyable.h"
#include<string>
// 定义日志的级别 INFO
enum LogLevel{
    INFO, // 信息
    DEBUG,  // 调试
    WARNING, //警告
    ERROR, //错误
    FATAL //毁灭性问题
};

#define LOG_INFO(msg,...)\
    do{\
        std::string s(msg);\
        if(s.size() < 1024){\
            Logger::instance().setLogLevel(INFO);\
            char buffer[1024]={0};\
            snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__);\
            std::string str(buffer);\
            Logger::instance().log(str);\
        }\
    }while(0)

#define LOG_WARNING(msg,...)\
    do{\
        std::string s(msg);\
        if(s.size() < 1024){\
            Logger::instance().setLogLevel(WARNING);\
            char buffer[1024]={0};\
            snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__);\
            std::string str(buffer);\
            Logger::instance().log(str);\
        }\
    }while(0)

#define LOG_ERROR(msg,...)\
    do{\
        std::string s(msg);\
        if(s.size() < 1024){\
            Logger::instance().setLogLevel(ERROR);\
            char buffer[1024]={0};\
            snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__);\
            std::string str(buffer);\
            Logger::instance().log(str);\
        }\
    }while(0)
    
#define LOG_FATAL(msg,...)\
    do{\
        std::string s(msg);\
        if(s.size() < 1024){\
            Logger::instance().setLogLevel(FATAL);\
            char buffer[1024]={0};\
            snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__);\
            std::string str(buffer);\
            Logger::instance().log(str);\
        exit(-1);\
        }\
    }while(0)

#ifdef MODUE_DEBUG
#define LOG_DEBUG(msg,...)\
    do{\
        if(strlen(msg) >= 1024) return;\
        Logger::instance().setLogLevel(DEBUG);\
        char buffer[1024]={0};\
        snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__);\
        std::string str(buffer);\
        Logger::instance().log(str);\
    }while(0)
#else
#define LOG_DEBUG(msg,...)
#endif

//输出一个日志类
class Logger:noncopyable{
public:
    static Logger& instance(){
        static Logger instance;
        return instance;
    };

    // 设置日志级别
    void setLogLevel(int level){
        logLevel_ = level;
    }

    // 日志输出函数
    void log(std::string& msg);
protected:
    // 日志级别
    int logLevel_;
};
    