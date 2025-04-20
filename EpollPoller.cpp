#include"EpollPoller.h"
#include"Logger.h"
#include"Channel.h"
#include<unistd.h>
#include<strings.h>
const int kNew = -1; //未添加的成员都是-1
const int kAdded = 1; //已添加
const int kDeleted = 2; //删除
EpollPoller::EpollPoller(EventLoop* loop):Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if(epollfd_ < 0){
        LOG_FATAL("EpollPoller::EpollPoller() - epoll_create1 error:%d",errno);
    }
};

EpollPoller::~EpollPoller(){
    ::close(epollfd_);
};

TimeStamp EpollPoller::poll(int timeoutMs,ChannleList* activeChannels){
    LOG_INFO("EpollPoller::poll() - timeoutMs:%d",timeoutMs);
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int savedErrno = errno;
    if(numEvents > 0){
        LOG_INFO("EpollPoller::poll() - %d events happened",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        /* 当前所监听的事件达到上限，就应该扩容 */
        if(numEvents == static_cast<int>(events_.size())){
            events_.resize(events_.size()*2);
        }else if(numEvents == 0){
            LOG_DEBUG("EpollPoller::poll() - no events happened");
        }else{
            /* 非中断 */
            if(savedErrno != EINTR){
                errno = savedErrno;
                LOG_ERROR("EpollPoller::poll() - epoll_wait error:%d",savedErrno);
            }
        }
    }
    
    return TimeStamp::now();
};

/* channel的update通过EventLoop ->poller updateChannel */
void EpollPoller::updateChannel(Channel* channel){
    if(!channel){
        return;
    }
    const int index = channel->index();
    LOG_INFO("EpollPoller::updateChannel() - index:%d",index);
    int fd = channel->fd();
    if(index == kNew || index == kDeleted){
        if(index == kNew){
            //添加channel
            channels_[fd] = channel;

        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else{
        //更新channel
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel){
    if(!channel){
        return;
    }

    int fd = channel->fd();
    channels_.erase(fd);
    int index = channel->index();
    if(index == kAdded){
        channel->set_index(kDeleted);
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(int numEvents,ChannleList* activeChannels)const{
    for(int i = 0;i<numEvents;i++){
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events); //将epoll_wait返回的事件赋值给channel
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation,Channel* channel){
    if(!channel){
        return;
    }

    epoll_event event;
    bzero(&event,sizeof(event)); //清空epoll_event结构体
    int fd = channel->fd();
    event.data.ptr = channel; //将channel指针放入epoll_event结构体中
    event.events = channel->events(); //将channel的事件放入epoll_event结构体中
    if(::epoll_ctl(epollfd_,operation,fd,&event) < 0){
        /* 区分下删除，修改，新增 */
        if(operation == EPOLL_CTL_DEL){
            LOG_ERROR("EpollPoller::update() - epoll_ctl op:%d fd:%d error:%d",operation,fd,errno);
        }else{
            LOG_FATAL("EpollPoller::update() - epoll_ctl op:%d fd:%d error:%d",operation,fd,errno);
        }
    }
}