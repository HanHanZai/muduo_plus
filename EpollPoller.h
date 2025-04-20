#pragma once
#include"Poller.h"
#include<vector>
#include<sys/epoll.h>
/**
 * Epoll的使用
 * Epoll_create 创建
 * Epoll_ctl 修改
 * Epoll_wait 等待
 */
class Channel;
class EpollPoller:public Poller{
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller()override;

    /* timeoutMs超时时间 activeChannels，被激活的channel，给所有IO复用保留统一接口*/
    TimeStamp poll(int timeoutMs,ChannleList* activeChannels)override; //返回活跃的channel
    /* 所有对channel的更新都只能在该loop线程中进行处理 */
    void updateChannel(Channel* channel)override; //更新channel
    void removeChannel(Channel* channel)override; //删除channel
private:
    static const int kInitEventListSize = 16; //epoll_event结构体数组的初始大小

    //填充活跃的channel
    void fillActiveChannels(int numEvents,ChannleList* activeChannels)const; 
    /* 更新channel通道 */
    void update(int operation,Channel* channel); //更新channel
    using EventList = std::vector<epoll_event>; //epoll_event结构体数组
    int epollfd_;   //epoll文件描述符
    EventList events_; //所有的监听事件对象
};