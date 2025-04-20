#pragma once
#include"nocopyable.h"
#include"Timestamp.h"
#include"CurrentThread.h"
#include<functional>
#include<atomic>
#include<memory>
#include<vector>
#include<mutex>
class Channel;
class Poller;

//时间循环类，主要包含了两个大模块channel和poller(epoll的抽象),has->a关系
class EventLoop:noncopyable{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop(); //事件循环
    void quit(); //退出事件循环
    void runInLoop(Functor cb); //在事件循环中执行cb
    void queueInLoop(Functor cb); //在事件循环中排队执行cb
    void wakeup(); //唤醒事件循环
    void updateChannel(Channel* channel); //更新channel
    void removeChannel(Channel* channel); //删除channel
    bool hasChannel(Channel* channel); //判断channel是否在poller中
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); } //判断当前线程是否是事件循环线程
    TimeStamp pollReturnTime() const { return pollReturnTime_; } //获取poller返回的时间
private: 

    void handleRead(); //处理读事件
    void doPendingFunctors(); //执行待处理的回调函数
    using ChannelList = std::vector<Channel*>; //保存所有的channel
    std::atomic<bool> looping_; //是否在事件循环中 原子操作，通过CAS实现
    std::atomic<bool> quit_; //是否退出事件循环
    const pid_t threadId_; //当前线程id
    TimeStamp pollReturnTime_; //poller返回的时间,每次poll后的时间点
    std::unique_ptr<Poller> poller_; //poller对象

    /* 主reactor进行交互 */
    int wakeupFd_; //唤醒fd,当mainLoop获取一个新用户的channel，通过轮询算法选择一个subLoop，通过该成员唤醒线程
    std::unique_ptr<Channel> wakeupChannel_; //唤醒channel

    ChannelList activeChannels_; //活跃的channel

    std::atomic<bool> callingPendingFunctors_; //是否在执行回调函数
    std::vector<Functor> pendingFunctors_; //待执行的回调函数 存储loop需要执行的所有回调操作
    std::mutex mutex_; //互斥锁，保护pendingFunctors_的线程安全
};