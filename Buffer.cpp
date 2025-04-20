#include"Buffer.h"
#include<sys/uio.h>
#include<unistd.h>
#include<errno.h>
/* 从fd上读取数据 LT模式水平触发，不会丢数据
   Buffer缓冲区是有大小的，但是tcp数据最终的数据大小
 */
ssize_t Buffer::readFd(int fd, int* savedErrno){
    char extraBuf[65536]; //额外的缓冲区 栈上的空间大小64k
    struct iovec vec[2]; //iovec结构体
    const size_t writable = writableBytes(); //可写字节数
    vec[0].iov_base = begin() + writeIndex_; //可写数据的首地址
    vec[0].iov_len = writable; //可写字节数
    vec[1].iov_base = extraBuf; //额外的缓冲区
    vec[1].iov_len = sizeof(extraBuf); //额外的缓冲区大小

    const int iovecCount = (writable < sizeof(extraBuf)) ? 2 : 1; //iovec的个数
    const ssize_t n = ::readv(fd, vec, iovecCount); //从fd中读取数据
    if(n < 0){
        *savedErrno = errno; //保存错误码
    }else if(static_cast<size_t>(n) <= writable){
        writeIndex_ += n; //更新写索引
    }else{
        writeIndex_ = buffer_.size(); //更新写索引
        append(extraBuf, n - writable); //将额外的缓冲区的数据拷贝到缓冲区
    }
    return n; //返回读取的字节数
}
 
ssize_t Buffer::writeFd(int fd,int* savedErrno){
    size_t n = writableBytes(); //可写字节数
    ssize_t nwrite = ::write(fd, peek(), n); //将数据写入fd
    if(nwrite < 0){
        *savedErrno = errno; //保存错误码
    }
    return nwrite; //返回写入的字节数
}