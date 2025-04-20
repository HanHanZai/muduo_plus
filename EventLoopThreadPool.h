#pragma once
#include"nocopyable.h"
#include"EventLoopThread.h"
#include<string>
#include<vector>
#include<memory>

class EventLoopThread;
class EventLoopThreadPool:noncopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop,const std::string& name = "");
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads){ numThreads_ = numThreads; } //设置线程数量
    void start(const ThreadInitCallback& cb = ThreadInitCallback()); //启动线程池
    EventLoop* getNextLoop(); //多线程中，baseloop默认以轮询的方式分配channel给subloop
    std::vector<EventLoop*> getAllLoops(){return loops_;}; //获取所有事件循环
    bool started() const { return started_; } //是否启动
    const std::string& name() const { return name_; } //获取线程池名称
    EventLoop* getBaseLoop() const { return baseloop_; } //获取主事件循环
private:
    EventLoop* baseloop_; //主reactor;
    std::string name_;
    bool started_; //是否启动
    int numThreads_; //线程数量
    int next_; //下一个线程
    std::vector<std::unique_ptr<EventLoopThread>> threads_; //线程池
    std::vector<EventLoop*> loops_; //subRector;
};