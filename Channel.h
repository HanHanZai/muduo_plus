#pragma once
#include"nocopyable.h"
#include"Timestamp.h"
#include<functional>
#include<memory>
/**
 * Channel类理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT等
 * 还绑定了poller返回的具体时间
 * 理清楚EventLoop、Channel、Poller之间的关系 <= 属于事件分发器
 */

class EventLoop;
class Channel:noncopyable{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;
    Channel(EventLoop* loop,int fd);
    ~Channel();

    //fd得到poller通知以后，处理事件的，调用响应的方法
    void handleEvent(TimeStamp receiveTime);
    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(EventCallback cb){
        writeCallback_ = std::move(cb);
    }
    void setCloseCallback(EventCallback cb){
        closeCallback_ = std::move(cb);
    }
    void setErrorCallback(EventCallback cb){
        errorCallback_ = std::move(cb);
    }

    //防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>& obj);
    int fd()const{return fd_;} //返回fd连接对象
    int events()const{return events_;} //返回感兴趣的事件
    int set_revents(int ev){
        revents_  = ev;
        return revents_;
    }

    void enableReading(){
        events_ |= kReadEvent;
        update();
    }

    void disableReading(){
        events_ &= ~kReadEvent;
        update();
    }

    void enableWriting(){
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting(){
        events_ &= ~kWriteEvent;
        update();
    }

    void disableAll(){
        events_ = kNoneEvent;
        update();
    }
    
    bool isWriting()const{
        return events_ & kWriteEvent;
    }
    bool isReading()const{
        return events_ & kReadEvent;
    }
    bool isNoneEvent()const{
        return events_ == kNoneEvent;
    }

    int index()const{
        return index_;
    }
    void set_index(int index){
        index_ = index;
    }
    
    EventLoop* ownerLoop()const{
        return loop_;
    }
    
    void remove();
private:
    /* 将当前fd感兴趣的事件更新到Poller */
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    EventLoop* loop_; //事件循环
    const int fd_; //Poller监听的对象
    int events_; //感兴趣的事件
    int revents_; //Poller返回的具体发生的事件
    int index_; //在poller中的索引
    std::weak_ptr<void> tie_; //绑定的资源对象
    bool tied_;
    ReadEventCallback readCallback_; //读事件回调
    EventCallback writeCallback_; //写事件回调
    EventCallback closeCallback_; //关闭事件回调
    EventCallback errorCallback_; //错误事件回调
};