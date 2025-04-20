#pragma once
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>
class InetAddress{
public:
    InetAddress();
    explicit InetAddress(uint16_t port,const char* ip="127.0.0.1");
    explicit InetAddress(sockaddr_in addr):addr_(addr){};
    InetAddress(uint16_t port);

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;
    const sockaddr* getSockAddr() const { return (const sockaddr*)&addr_; }
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }
private:
    sockaddr_in addr_;
};