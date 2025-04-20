#include"Socket.h"
#include"InetAddress.h"
#include"Logger.h"
#include<unistd.h>
#include<strings.h>
#include<sys/socket.h>
#include<netinet/tcp.h>
Socket::Socket(int sockfd):sockfd_(sockfd){

}

Socket::~Socket(){
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr){
    if (::bind(sockfd_, localaddr.getSockAddr(), sizeof(struct sockaddr_in)) < 0){
        LOG_FATAL("bind error，address:%s", localaddr.toIpPort().c_str());
    }
}

void Socket::listen(){
    if (::listen(sockfd_, 1024) < 0){
        LOG_FATAL("listen socket error,sockfd:%d", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr){
    /**
     * 是个非阻塞，I/O多路复用的socket
     */
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept4(sockfd_, (struct sockaddr*)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0){
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::connect(InetAddress* peeraddr){
    int ret = ::connect(sockfd_, peeraddr->getSockAddr(), sizeof(struct sockaddr_in))
    if (ret < 0){
        LOG_FATAL("connect error, sockfd:%d", sockfd_);
    }
}

void Socket::shutdownWrite(){
    if (::shutdown(sockfd_, SHUT_WR) < 0){
        LOG_ERROR("shutdown write error, sockfd:%d", sockfd_);
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) < 0){
        LOG_ERROR("set TCP_NODELAY error, sockfd:%d", sockfd_);
    }
}

void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
        LOG_ERROR("set SO_REUSEADDR error, sockfd:%d", sockfd_);
    }
}

void Socket::setReusePort(bool on){
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0){
        LOG_ERROR("set SO_REUSEPORT error, sockfd:%d", sockfd_);
    }
}

void Socket::setKeepAlive(bool on){
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0){
        LOG_ERROR("set SO_KEEPALIVE error, sockfd:%d", sockfd_);
    }
}