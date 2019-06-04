//
// Created by wuxiaofeng on 2019/6/4.
//
#pragma once

#include <iostream>
#include "vendor_boost.h"

using TIMEOUT_HANDLE = std::function<bool (void)>;

class timer_actor : public std::enable_shared_from_this<timer_actor> {
public:
    timer_actor(boost::asio::io_context &io, uint32_t sec, TIMEOUT_HANDLE func);
    ~timer_actor();

    void start();

    void handle(TIMEOUT_HANDLE func);

    void renew(uint32_t sec);

    void cancel();

private:
    void on_timer(const boost::system::error_code &ec);

    uint32_t sec_;
    boost::asio::deadline_timer     timer_;
    std::function<bool (void)>      handle_;
};
