#include"Thread.h"
#include"CurrentThread.h"
#include<semaphore.h>
std::atomic<int> Thread::numCreated_(0);
Thread::Thread(ThreadFunc func, const std::string& name):
    started_(false),
    joined_(false),
    thread_(nullptr),
    tid_(0),
    func_(std::move(func)),
    name_(name){
    setDefaultName();
}

Thread::~Thread(){
    if(started_ && !joined_){
        thread_->detach(); //主线程结束后，子线程也结束
    }
}

void Thread::start(){
    started_ = true;
    sem_t sem;
    sem_init(&sem, 0, 0);
    /* 开启线程 */
    thread_ = std::make_shared<std::thread>([&]{
        tid_ = CurrentThread::tid();
        sem_post(&sem); //子线程创建完成，通知主线程
        func_();
    });

    sem_wait(&sem); //等待子线程创建完成
}

void Thread::join(){
    joined_ = true;
    thread_->join(); //主线程等待子线程结束
}

void Thread::setDefaultName(){
    int num = ++numCreated_;
    if (name_.empty()){
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}