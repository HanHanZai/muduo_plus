#include"EventLoopThreadPool.h"
#include"EventLoop.h"
#include"EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,const std::string& name):
    baseloop_(baseLoop),
    name_(name),
    started_(false),
    numThreads_(0),
    next_(0){
    if (name_.empty()){
        name_ = "EventLoopThreadPool";
    }
}

EventLoopThreadPool::~EventLoopThreadPool(){
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb){
    started_ = true;
    for (int i = 0; i < numThreads_; ++i){
        EventLoopThread* thread = new EventLoopThread(cb, name_ + std::to_string(i + 1));
        threads_.emplace_back(thread);
        loops_.emplace_back(thread->startLoop());
    }

    /* 只有一个线程进行处理 */
    if(numThreads_ == 0 && cb){
        cb(baseloop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    EventLoop* loop = baseloop_;
    if (loops_.size() > 0){
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()){
            next_ = 0;
        }
    }
    return loop;
}