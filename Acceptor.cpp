#include"Acceptor.h"
#include"Logger.h"
#include"InetAddress.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
//SOCK_CLOEXEC 子进程不继承父进程的sockfd
static int createNonblocking(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0){
        LOG_FATAL("createNonblocking error");
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport):
    loop_(loop),
    acceptSocket_(createNonblocking()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false)
{
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this)); 
}

Acceptor::~Acceptor(){
    acceptChannel_.disableAll(); //关闭所有事件
    acceptChannel_.remove(); //从poller中删除
    ::close(acceptSocket_.fd()); //关闭套接字
    listening_ = false;
}

void Acceptor::listen(){
    acceptSocket_.listen(); //监听套接字
    listening_ = true; //设置为监听状态
    acceptChannel_.enableReading(); //启用读事件
}

void Acceptor::handleRead(){
    InetAddress peeraddr;
    int connfd = acceptSocket_.accept(&peeraddr);
    if(connfd >= 0){
        /* 这个回调在TcpServer中选择一个subReactor来进行事件处理 */
        if(newConnectionCallback_){
            newConnectionCallback_(connfd, peeraddr);
        }else{
            ::close(connfd);
        }
    }else{
        LOG_ERROR("accept error");
        if(errno == EMFILE){
            LOG_ERROR("EMFILE"); //文件描述符已经达到上限了
        }
    }
}