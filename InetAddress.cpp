#include<strings.h>
#include"InetAddress.h"
InetAddress::InetAddress(){

};

InetAddress::InetAddress(uint16_t port,const char* ip){
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET; 
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip);
}


InetAddress::InetAddress(uint16_t port){
    const char* ip = "127.0.0.1";
    InetAddress(port,ip);
}

std::string InetAddress::toIp() const{
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const{
    char buf[64]={0};
    snprintf(buf,64,"%s:%u",toIp().c_str(),toPort());
    return buf;
}

uint16_t InetAddress::toPort() const{
    return ntohs(addr_.sin_port);
}