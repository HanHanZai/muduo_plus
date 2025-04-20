#include"TcpConnection.h"
#include"EventLoop.h"
#include"Buffer.h"
#include"Logger.h"
#include"Socket.h"
#include"Channel.h"
#include<sys/socket.h>
static EventLoop* CheckLoopNotNull(EventLoop* loop){
    if (loop == nullptr){
        LOG_FATAL("TcpConnection is null %s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,const std::string& nameArg,
                int sockfd,const InetAddress& localAddr,const InetAddress& peerAddr):
                loop_(CheckLoopNotNull(loop)),
                name_(nameArg),
                state_(kConnecting),
                socket_(new Socket(sockfd)), //创建socket对象
                channel_(new Channel(loop,sockfd)), //创建channel对象
                localAddr_(localAddr), //本地地址
                peerAddr_(peerAddr), //对端地址
                reading_(true), //是否正在读取数据
                highWaterMark_(64*1024*1024) //高水位线{
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1)); //设置读回调函数
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this)); //设置写回调函数
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this)); //设置关闭回调函数
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this)); //设置错误回调函数

    LOG_DEBUG("TcpConnection::ctor[%s] at %p fd=%d", name_.c_str(), this, sockfd); //打印连接信息
    socket_->setKeepAlive(true); //设置长连接
}

TcpConnection::~TcpConnection(){
}

//发送数据
void TcpConnection::send(const void* data, size_t len){
    if(state_ == kConnected){ //如果连接状态为连接中
        if(loop_->isInLoopThread()){ //如果在事件循环线程中
            sendInLoop(data, len); //发送数据
        }else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, data, len)); //将发送数据加入事件循环
        }
    }
}

//关闭连接
void TcpConnection::shutdown(){
    if(state_ == kConnected){ //如果连接状态为连接中
        setState(kDisconnecting); //设置状态为断开连接
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this)); //将关闭连接加入事件循环
    }
}

/* 连接建立 */
void TcpConnection::connectionEstablished(){
    setState(kConnected); //设置状态为连接中
    channel_->tie(shared_from_this()); //将channel与TcpConnection绑定
    channel_->enableReading(); //启用读事件
    connectionCallback_(shared_from_this()); //调用连接回调函数
    LOG_DEBUG("TcpConnection::connectionEstablished[%s] fd=%d", name_.c_str(), channel_->fd()); //打印连接建立信息
}

/* 连接断开 */
void TcpConnection::connectionDestroyed(){
    if(state_ == kConnected){ //如果连接状态为连接中
        setState(kDisconnected); //设置状态为断开连接
        channel_->disableAll(); //禁用所有事件
        connectionCallback_(shared_from_this()); //调用连接回调函数
    }
    LOG_DEBUG("TcpConnection::connectionDestroyed[%s] fd=%d", name_.c_str(), channel_->fd()); //打印连接断开信息
    channel_->remove(); //从poller中移除channel
}

void TcpConnection::handleRead(TimeStamp receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno); //从fd中读取数据
    if(n > 0){
        /* 建立连接的用户发生了可读取事件 */
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime); //调用消息回调函数
    }
    else if(n == 0){
        handleClose();
    }else{
        savedErrno = errno;
        LOG_ERROR("TcpConnection::handleRead [%s] errno = %d", name_.c_str(), savedErrno);
        handleError(); 
    }
}

void TcpConnection::handleWrite(){
    if(channel_->isWriting()){
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno); //将数据写入fd
        if(n > 0){
            outputBuffer_.retrieve(n); //清空缓冲区
            if(outputBuffer_.readableBytes() == 0){ //如果缓冲区为空
                channel_->disableWriting(); //禁用写事件
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this())); //将写完成回调函数加入事件循环
                }
                if(state_ == kDisconnecting){ //如果正在断开连接
                    shutdownInLoop(); //关闭连接
                }
            }
        }
    }else{
        LOG_ERROR("Connection is down, no more writing");
    }
}

void TcpConnection::handleClose(){
    setState(kDisconnected); //设置状态为断开连接
    channel_->disableAll(); //禁用所有事件
    TcpConnectionPtr connPtr(shared_from_this()); //创建TcpConnectionPtr对象
    connectionCallback_(connPtr); //调用连接回调函数
    closeCallback_(connPtr); //调用关闭回调函数
}

void TcpConnection::handleError(){
    int optval = 0;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        err = errno; //保存错误码
        LOG_ERROR("TcpConnection::handleError [%s] SO_ERROR = %d", name_.c_str(), optval); //打印错误信息
    }else{
        err = optval;
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len){
    ssize_t n = 0;
    size_t remaining = len; //剩余字节数
    bool faultError = false; //是否发生错误
    if(state_ == kDisconnected){ //如果连接状态为断开连接
        LOG_ERROR("disconnected, give up writing");
        return;
    }

    /* 第一次写数据 */
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0){ //如果没有写事件并且缓冲区为空
        n = ::send(channel_->fd(), data, len, MSG_NOSIGNAL); //发送数据
        if(n < 0){
            remaining = len; //剩余字节数
            if(errno != EWOULDBLOCK){ //如果不是阻塞错误
                LOG_ERROR("TcpConnection::sendInLoop [%s] sendsize=%zd", name_.c_str(), n);
                if(n < 0){
                    faultError = true; //发生错误
                }
            }
        }else{
            remaining -= n;
            /* 如果完全发送完了 */
            if(remaining == 0 && writeCompleteCallback_){
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this())); //将写完成回调函数加入事件循环
            }
        }
    }

    if(remaining > 0){ //如果还有剩余字节数
        outputBuffer_.append(static_cast<const char*>(data) + n, remaining); //将数据添加到缓冲区
        if(!channel_->isWriting()){ //如果没有写事件
            channel_->enableWriting(); //启用写事件
        }
    }
}

void TcpConnection::shutdownInLoop(){
    loop_->removeChannel(channel_.get()); //从事件循环中移除channel
    channel_->disableAll(); //禁用所有事件
    channel_->remove(); //从poller中移除channel
    socket_->shutdownWrite(); //关闭写端
    if(closeCallback_){
        loop_->queueInLoop(std::bind(closeCallback_, shared_from_this())); //将关闭回调函数加入事件循环
    }
    state_ = kDisconnected; //设置状态为断开连接
    LOG_DEBUG("TcpConnection::shutdownInLoop[%s] fd=%d", name_.c_str(), channel_->fd()); //打印关闭连接信息
}

void TcpConnection::send(const std::string& buf){
    if(state_ == kConnected){ //如果连接状态为连接中
        if(loop_->isInLoopThread()){ //如果在事件循环线程中
            sendInLoop(buf.c_str(), buf.size()); //发送数据
        }else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size())); //将发送数据加入事件循环
        }
    }
}