#pragma once
#include"nocopyable.h"
#include"Timestamp.h"
#include"Callbacks.h"
#include"InetAddress.h"
#include"Buffer.h"
#include<memory>
#include<string>
#include<atomic>
class Channel;
class EventLoop;
class Socket;
/* 具体连接执行的回调函数 */
/**
 *  TcpServer->Acceptor->通过accepet函数拿到connfd->TcpConnection
 */
class TcpConnection:noncopyable,public std::enable_shared_from_this<TcpConnection>{
public:
    enum StateE{
        kDisconnected, //断开连接
        kConnecting, //正在连接
        kConnected, //连接成功
        kDisconnecting //正在断开连接
    };

    TcpConnection(EventLoop* loop,
                  const std::string& nameArg,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    //发送数据
    void send(const void* data, size_t len);
    //关闭连接
    void shutdown();

    /* 连接建立 */
    void connectionEstablished();
    /* 连接断开 */
    void connectionDestroyed();

    EventLoop* getLoop()const{
        return loop_;
    }
    const std::string& name()const{
        return name_;
    }
    const InetAddress& localAddress()const{
        return localAddr_;
    }
    const InetAddress& peerAddress()const{
        return peerAddr_;
    }
    bool connected()const{
        return state_ == kConnected;
    }
    void setConnectionCallback(const ConnectionCallback& cb){
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb){
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark){
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb){
        closeCallback_ = cb;
    }
    void send(const std::string& buf); //发送数据
private:
    void setState(StateE s){
        state_ = s;
    }

    void handleRead(TimeStamp receiveTime); //处理读事件
    void handleWrite(); //处理写事件
    void handleClose(); //处理关闭事件
    void handleError(); //处理错误事件
    void sendInLoop(const void* data, size_t len); //在loop中发送数据
    void shutdownInLoop(); //在loop中关闭连接
    EventLoop* loop_; //连接对应的哪个loop对象
    const std::string name_; //连接的名字
    std::atomic<int> state_;
    bool reading_; //是否正在读取数据
    std::unique_ptr<Socket> socket_; //对应fd创建的socket
    std::unique_ptr<Channel> channel_; //对应fd创建的channel
    const InetAddress localAddr_; //本地地址 当前服务器地址
    const InetAddress peerAddr_; //远程地址 远端服务器地址
    ConnectionCallback connectionCallback_; //连接成功的回调函数
    MessageCallback messageCallback_; //消息到达的回调函数
    WriteCompleteCallback writeCompleteCallback_; //写完成的回调函数
    HighWaterMarkCallback highWaterMarkCallback_; //高水位线的回调函数
    CloseCallback closeCallback_; //关闭连接的回调函数
    size_t highWaterMark_; //高水位线
    Buffer inputBuffer_; //接受数据缓冲区
    Buffer outputBuffer_; //发送数据缓冲区
};
