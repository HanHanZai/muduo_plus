#pragma once
#include"nocopyable.h"
#include<vector>
#include<string>
/* 网络库底层的缓冲器类型定义 */
class Buffer:noncopyable{
public:
    static const size_t kCheapPrepend = 8; //预留8个字节
    static const size_t kInitialSize = 1024; //初始大小为1024字节
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), //预留8个字节
          readIndex_(kCheapPrepend), //读索引
          writeIndex_(kCheapPrepend) //写索引
          {

          }
    size_t readableBytes() const {
        return writeIndex_ - readIndex_; //可读字节数
    }

    size_t writableBytes() const {
        return buffer_.size() - writeIndex_; //可写字节数
    }

    /* 预留的数据头 */
    size_t prependableBytes() const {
        return readIndex_; //可预留字节数
    }    

    //返回缓冲区中可读取的起始地址
    const char* peek()const{
        return begin() + readIndex_; //返回可读数据的首地址
    }

    void retrieve(size_t len){
        if(len < readableBytes()){
            readIndex_ += len; //更新读索引
        }else{
            retrieveAll(); //清空缓冲区
        }
    }

    void retrieveAll(){
        readIndex_ = kCheapPrepend; //清空缓冲区
        writeIndex_ = kCheapPrepend; //清空缓冲区
    }

    /* 读取全部的数据 */
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes()); //返回可读数据
    }

    /* 获取指定大小的数据 */
    std::string retrieveAsString(size_t len){
        std::string result(peek(),len); //返回可读数据
        retrieve(len); //清空缓冲区
        return result; //返回可读数据
    }

    void ensureWritableBytes(size_t len){
        if(writableBytes() < len){ //如果可写字节数小于len
            makeSpace(len); //扩展缓冲区
        }
    }

    void makeSpace(size_t len){
        if(writableBytes() + prependableBytes() < len + kCheapPrepend){ //如果可写字节数小于len
            buffer_.resize(writeIndex_ + len); //扩展缓冲区
        }else{
            size_t readable = readableBytes(); //可读字节数
            std::copy(begin() + readIndex_, begin() + writeIndex_, begin() + kCheapPrepend); //将可读数据拷贝到缓冲区的开头
            readIndex_ = kCheapPrepend; //更新读索引
            writeIndex_ = readIndex_ + readable; //更新写索引
        } 
    }

    void append(const char* data, size_t len){
        ensureWritableBytes(len); //确保可写字节数大于len
        std::copy(data, data + len, beginWrite()); //将数据拷贝到缓冲区
        writeIndex_ += len; //更新写索引
    }
    
    /* 从fd中读取数据 */
    ssize_t readFd(int fd, int* savedErrno);
    ssize_t writeFd(int fd,int* savedErrno);
private:
    char* begin(){
        return &*buffer_.begin(); //返回缓冲区的首地址
    }

    const char* begin() const {
        return &*buffer_.begin(); //返回缓冲区的首地址
    }

    char* beginWrite(){
        return begin() + writeIndex_; //返回可写数据的首地址
    }
    const char* beginWrite() const {
        return begin() + writeIndex_; //返回可写数据的首地址
    }

    std::vector<char> buffer_; //缓冲区
    size_t readIndex_; //读索引
    size_t writeIndex_; //写索引
};