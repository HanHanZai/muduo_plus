#include"Poller.h"
#include"Channel.h"

Poller::Poller(EventLoop* loop):ownerLoop_(loop){

}

Poller::~Poller(){
}

bool Poller::hasChannel(Channel* channel) const{
    if(!channel){
        return false;
    }
    auto itor = channels_.find(channel->fd());
    return itor != channels_.end() && itor->second == channel;
}
