//
// Created by wuxiaofeng on 2019/5/24.
//
#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class local_addr {
public:
    local_addr();
    operator std::string();
    operator sockaddr();
    ~local_addr();
private:
    struct ifaddrs* addrs_;
};
