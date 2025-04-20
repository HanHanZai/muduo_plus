#pragma once
#include"nocopyable.h"
#include"Socket.h"
#include"Channel.h"
#include<functional>
class EventLoop;
class Acceptor:noncopyable{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>; //新连接回调函数
    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport=true);
    ~Acceptor();
    void listen(); //开始监听
    void setNewConnectionCallback(const NewConnectionCallback& cb) { //设置新连接回调函数
        newConnectionCallback_ = cb;
    }
    bool listening() const { return listening_; } //是否在监听
private:
    void handleRead(); //处理读事件
    EventLoop* loop_; //用的就是用户定义的baseloop
    Socket acceptSocket_; //对应的连接套接字相关内容
    Channel acceptChannel_; //处理连接的对象
    NewConnectionCallback newConnectionCallback_; //新连接回调函数
    bool listening_; //是否在监听
};