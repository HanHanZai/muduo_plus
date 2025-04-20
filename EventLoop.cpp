#include"EventLoop.h"
#include"Channel.h"
#include"Logger.h"
#include"EpollPoller.h"
#include<sys/eventfd.h>
//防止一个线程创建多个EventLoop对象,线程单例对象?
const int kPollTimeMs = 10000; //poller的超时时间
__thread EventLoop* t_loopInThisThread = 0; //当前线程的事件循环

/* 主reactor通知subreactor工作 */
int createEventFd(){
    int evtFd = eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtFd < 0){
        LOG_FATAL("Failed in eventfd");
    }
    return evtFd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()),
    pollReturnTime_(),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventFd()),
    wakeupChannel_(new Channel(this,wakeupFd_)),
    callingPendingFunctors_(false)
{
    LOG_DEBUG("EventLoop created %p in thread %d",this,threadId_);
    if(t_loopInThisThread){
        LOG_FATAL("Another EventLoop %p exists in this thread %d",t_loopInThisThread,threadId_);
    }else{
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();

}

EventLoop::~EventLoop(){
    wakeupChannel_->disableAll(); 
    wakeupChannel_->remove(); //从poller中删除
}

void EventLoop::loop(){
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping",this);
    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_); //阻塞等待事件发生
        for(auto& channel : activeChannels_){
            channel->handleEvent(pollReturnTime_); //处理事件
        }
        //执行当前EventLoop事件循环需要处理的回调ca
        doPendingFunctors(); //执行待处理的回调函数
    }
} 

void EventLoop::quit(){ 
    quit_ = true;
    /* 可能是其他线程执行这个操作 */
    if(!isInLoopThread()){
        wakeup(); //唤醒EventLoop::loop()函数
    }
}

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){
        cb(); //如果当前线程是事件循环线程，直接执行
    }else{
        /* 唤醒loop所在线程中执行cb */
        queueInLoop(cb); //否则加入到待执行的回调函数中
    }
}

void EventLoop::queueInLoop(Functor cb){
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb); //将cb加入到待执行的回调函数中 
    }

    //callingPendingFunctors_ 在正在执行回调时，又来了回调
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup(); //唤醒loop所在线程
    }
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_,&one,sizeof(one)); //写入wakeupfd
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes instead of 8",n);
    }
}

void EventLoop::updateChannel(Channel* channel){
    poller_->updateChannel(channel); //更新poller中的channel
}

void EventLoop::removeChannel(Channel* channel){
    poller_->removeChannel(channel); //从poller中删除channel
}

bool EventLoop::hasChannel(Channel* channel){
   return poller_->hasChannel(channel); //判断poller中是否有channel
}

/* 执行回调 */
void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true; //正在执行待处理的回调函数
    {
        /* 锁的粒度不一样 */
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_); //交换两个vector
    }
    for(auto& fucntor: functors){
        fucntor(); //执行回调函数
    }
    callingPendingFunctors_ = false; //执行完毕
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8",n);
    }
}
