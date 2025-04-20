#pragma once
#include"nocopyable.h"
#include<thread>
#include<functional>
#include<memory>
#include<unistd.h>
#include<atomic>
#include<string>
class Thread:noncopyable{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc func, const std::string& name="");
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_; //线程id
    ThreadFunc func_;
    std::string name_;
    static std::atomic<int> numCreated_; //总共启动的线程数量统计
};