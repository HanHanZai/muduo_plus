#include"EventLoopThread.h"
#include"EventLoop.h"
EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
const std::string& name):
    loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(),
    callback_(cb){
}

EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if (loop_ != nullptr){
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop(){
    /* 启动线程 */
    thread_.start();
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&]{ return loop_ != nullptr; }); //等待条件变量
        loop = loop_;
    }

    return loop;
}

/* 单独的新线程中运行的函数 */
void EventLoopThread::threadFunc(){
    /* 创建一个独立的eventLoop,和上面的线程是一一对应的,one loop per thread */
    EventLoop loop;
    if(callback_){
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one(); //通知主线程
    }

    /* 正式进行时间循环 */
    loop.loop(); //事件循环
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }
}