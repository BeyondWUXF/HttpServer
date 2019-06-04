//
// Created by wuxiaofeng on 2019/6/4.
//
#pragma once

#include "timer_actor.h"

class timer_manager {
public:
    timer_manager(boost::asio::io_context &io);
    virtual ~timer_manager();
    static timer_manager* get() {return ins_;};

    uint64_t add_timer(uint32_t sec, TIMEOUT_HANDLE f);
    void del_timer(uint64_t timer_id);

private:
    uint64_t get_id() {return ++timer_seq_;}
    void clear();

private:
    static timer_manager* ins_;

    boost::asio::io_context &io_;
    boost::shared_mutex sm_;
    std::unordered_map<uint64_t, std::shared_ptr<timer_actor> > timers_;

    static uint64_t timer_seq_;
};

