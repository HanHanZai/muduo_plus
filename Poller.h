#pragma once
#include"nocopyable.h"
#include"Timestamp.h"
#include<vector>
#include<unordered_map>
class EventLoop;
class Channel;

/* muduo库中I/O多路复用的核心对象 */
class Poller:noncopyable{
public:
    using ChannleList = std::vector<Channel*>; //保存所有的channel
    Poller(EventLoop* loop);
    virtual ~Poller();
    
    /* timeoutMs超时时间 activeChannels，被激活的channel，给所有IO复用保留统一接口*/
    virtual TimeStamp poll(int timeoutMs,ChannleList* activeChannels) = 0; //返回活跃的channel
    /* 所有对channel的更新都只能在该loop线程中进行处理 */
    virtual void updateChannel(Channel* channel) = 0; //更新channel
    virtual void removeChannel(Channel* channel) = 0; //删除channel
    virtual bool hasChannel(Channel* channel) const; //判断channel是否在poller中
    
    static Poller* newDefaultPoller(EventLoop* loop); //创建poller对象
protected:
    using ChannelMap = std::unordered_map<int,Channel*>; //fd和channel的映射
    EventLoop* ownerLoop_; //事件循环
    ChannelMap channels_;
};