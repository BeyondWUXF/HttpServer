//
// Created by wuxiaofeng on 2019/5/24.
//
#include "local_addr.h"

local_addr::local_addr() {
    if(getifaddrs(&addrs_) != 0) {
        throw std::runtime_error("failed to get interface addresses");
    }
}

local_addr::operator std::string() {
    sockaddr addr = static_cast<sockaddr>(*this);
    if(addr.sa_family == AF_INET) {
        // std::string raddr(16, '\0');
        char buffer[16];
        sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(&addr);
        inet_ntop(addr4->sin_family, &(addr4->sin_addr), buffer, sizeof(buffer));
        return std::string(buffer);
    }else if(addr.sa_family == AF_INET6) {
        // std::string raddr(40, '\0');
        char buffer[40];
        sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(&addr);
        inet_ntop(addr6->sin6_family, &(addr6->sin6_addr), buffer, sizeof(buffer));
        return std::string(buffer);
    }else{
        throw std::runtime_error("failed to get interface addresses");
    }
}

static bool is_internal(sockaddr_in* addr) {
    return (addr->sin_addr.s_addr & 0x000000ff) == 0x0000000a // 10.0.0.0/8
           || (addr->sin_addr.s_addr & 0x00000fff) == 0x0000c10a // 172.16.0.0/112
           || (addr->sin_addr.s_addr & 0x0000ffff) == 0x0000a8c0; // 192.168.0.0/16
}

local_addr::operator sockaddr() {
    sockaddr raddr;
    raddr.sa_family = AF_UNSPEC;
    for(ifaddrs* addr = addrs_; addr != nullptr; addr = addr->ifa_next) {
        // std::printf("name: %s family: %08x\n", addr->ifa_name, addr->ifa_addr->s_addr);
        switch(addr->ifa_addr->sa_family) {
            case AF_INET:
                if(is_internal(reinterpret_cast<sockaddr_in*>(addr->ifa_addr))) {
                    raddr = *(addr->ifa_addr);
                }
                break;
            case AF_INET6:
            default:
                continue;
        }
    }
    return raddr;
}

local_addr::~local_addr() {
    freeifaddrs(addrs_);
}
