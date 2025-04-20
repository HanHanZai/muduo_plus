#include"Channel.h"
#include"Logger.h"
#include"EventLoop.h"
#include<sys/epoll.h>
const int Channel::kNoneEvent = 0; //无事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //读取事件
const int Channel::kWriteEvent = EPOLLOUT; //写入事件

/* EventLoop拥有loop */
Channel::Channel(EventLoop* loop,int fd)
    :loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
{

}

Channel::~Channel(){
    /* 确保当前channel是在对应线程中进行析构 */
    // if(loop_->isInLoopThread()){
    //     assert(!loop_->hasChannel(this));
    // }
}

/* 啥时候调用过这个tie */
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_ = obj;
    tied_ = true;
}

/* 更新当前channel所感兴趣的事件 
   EventLoop => poller + channel
*/
void Channel::update(){
    loop_->updateChannel(this);
}

/* 在channel所属的EventLoop中，把当前的channel删除掉 */
void Channel::remove(){
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp receiveTime){
    std::shared_ptr<void> guard;
    if(tied_){
        guard = tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}   

void Channel::handleEventWithGuard(TimeStamp receiveTime){
    LOG_INFO("channel handlEvent() revents:%d",revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        //发生了挂断事件
        if(closeCallback_){
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR){
        //发生了错误事件
        if(errorCallback_){
            errorCallback_();
        }
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        //发生了可读事件
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT){
        //发生了可写事件
        if(writeCallback_){
            writeCallback_();
        }
    }
}