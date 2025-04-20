#pragma once
#include "nocopyable.h"
class InetAddress;
//封装Socket fd
class Socket:noncopyable{
public:
    explicit Socket(int sockfd);
    ~Socket();
    int fd() const { return sockfd_; } //获取socket fd
    void bindAddress(const InetAddress& localaddr); //绑定地址
    void listen(); //监听
    void connect(InetAddress* peeraddr); //连接
    int accept(InetAddress* peeraddr); //接受连接
    void shutdownWrite(); //关闭写端
    void setTcpNoDelay(bool on); //设置TCP_NODELAY
    void setReuseAddr(bool on); //设置SO_REUSEADDR
    void setReusePort(bool on); //设置SO_REUSEPORT
    void setKeepAlive(bool on); //设置SO_KEEPALIVE
private:
    const int sockfd_;
};