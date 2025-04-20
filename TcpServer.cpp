#include"TcpServer.h"
#include"Logger.h"
#include"TcpConnection.h"
#include<strings.h>
static EventLoop* CheckLoopNotNull(EventLoop* loop){
    if (loop == nullptr){
        LOG_FATAL("mainLoop is null %s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg,Option option):
    loop_(CheckLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(),
    messageCallback_(),
    writeCompleteCallback_(),
    closeCallback_(),
    threadInitCallback_(),
    started_(0),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));    
    threadPool_->setThreadNum(0);
}

TcpServer::~TcpServer(){
    LOG_DEBUG("TcpServer::~TcpServer [%s] - %s", name_.c_str(), ipPort_.c_str()); //打印TcpServer析构函数信息
    for(auto& item : connections_){ 
        TcpConnectionPtr conn(item.second); //创建TcpConnectionPtr对象
        item.second.reset(); //重置连接
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectionDestroyed, conn)); //在事件循环中断开连接
    }
}

void TcpServer::start(){
    /* 防止TcpServer被重复启动多次 */
    if(started_++ == 0){ 
        threadPool_->start(threadInitCallback_); //启动底层线程池
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get())); //在主线程中启动
    }
}

void TcpServer::setThreadNum(int numThreads){
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd, const InetAddress& peeraddr){
    /* 选择一个subLoop来管理对应的channel */
    EventLoop* ioLoop = threadPool_->getNextLoop(); //获取下一个事件循环
    char buf[32];
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_); //生成连接id
    ++nextConnId_; //连接id自增
    std::string connName = name_ + buf; //连接名称
    sockaddr_in local;
    bzero(&local, sizeof(local)); //清空本地地址
    socklen_t addrlen = sizeof(local);
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0){ //获取本地地址
        LOG_ERROR("sockets::getLocalAddr failed");
    }
    InetAddress localAddr(local); //本地地址
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peeraddr)); //创建TcpConnection对象
    connections_[connName] = conn; //将连接加入map
    conn->setConnectionCallback(connectionCallback_); //设置连接回调函数
    conn->setMessageCallback(messageCallback_); //设置消息回调函数
    conn->setWriteCompleteCallback(writeCompleteCallback_); //设置写完成回调函数
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); //设置关闭回调函数
    ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn)); //在事件循环中建立连接
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn)); //在事件循环中移除连接
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
    connections_.erase(conn->name()); //从map中删除连接
    /* 和主线程不在一个线程 */
    EventLoop* ioLoop = conn->getLoop(); //获取事件循环
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn)); //在事件循环中断开连接
}