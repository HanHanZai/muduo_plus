#pragma once

/**
 * 用户使用muduo编写服务器程序
 */
#include"EventLoop.h"
#include"EventLoopThreadPool.h"
#include"Acceptor.h"
#include"Callbacks.h"
#include"InetAddress.h"
#include"nocopyable.h"
#include"EventLoop.h"
#include"Callbacks.h"
#include"Poller.h"
#include"TcpConnection.h"
#include"Timestamp.h"
#include<functional>
#include<string>
#include<memory>
#include<atomic>
#include<unordered_map>
/* 最终对外的类对象 */
class TcpServer:noncopyable{
public:
    enum Option{
        kNoReusePort, //不重用端口
        kReusePort, //重用端口
    };
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg,Option option = kNoReusePort);
    ~TcpServer();
    void start(); //启动服务器
    void setThreadNum(int numThreads); //设置线程池的线程数

    void setThreadInitCallback(const ThreadInitCallback& cb) { //设置线程初始化回调函数
        threadInitCallback_ = cb;
    }
    void setConnectionCallback(const ConnectionCallback& cb) { //设置连接回调函数
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb) { //设置消息回调函数
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { //设置写完成回调函数
        writeCompleteCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback& cb) { //设置关闭连接回调函数
        closeCallback_ = cb;
    }
private:
    void newConnection(int sockfd, const InetAddress& peeraddr); //新连接
    void removeConnection(const TcpConnectionPtr& conn); //删除连接
    void removeConnectionInLoop(const TcpConnectionPtr& conn); //在事件循环中删除连接

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>; // 连接map
    EventLoop* loop_; // 主事件循环,baseLoop
    const std::string ipPort_; // ip+port
    const std::string name_; // 服务器名字
    std::unique_ptr<Acceptor> acceptor_; // 运行在mainLoop，监听新连接
    std::unique_ptr<EventLoopThreadPool> threadPool_; // 线程池 one loop per thread
    
    ConnectionCallback connectionCallback_; // 连接建立回调
    MessageCallback messageCallback_; // 消息回调
    WriteCompleteCallback writeCompleteCallback_; // 写完成回调
    CloseCallback closeCallback_; // 关闭连接回调
    ThreadInitCallback threadInitCallback_; // 线程初始化回调
    std::atomic<int> started_; // 是否启动
    int nextConnId_; // 下一个连接id
    ConnectionMap connections_; // 所有的连接集合
};  
