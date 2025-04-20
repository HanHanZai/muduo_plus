#pragma once
#include"nocopyable.h"
#include"Thread.h"
#include<mutex>
#include<condition_variable>
class EventLoop;
class EventLoopThread:noncopyable{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = "");
    ~EventLoopThread();

    EventLoop* startLoop(); //启动事件循环
private:
    void threadFunc(); //线程函数
    EventLoop* loop_;
    bool exiting_; //是否退出
    Thread thread_; //线程对象
    std::mutex mutex_; //互斥锁
    std::condition_variable cond_; //条件变量
    ThreadInitCallback callback_; //线程初始化回调函数
};