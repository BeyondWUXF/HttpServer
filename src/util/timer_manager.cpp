//
// Created by wuxiaofeng on 2019/6/4.
//

#include "timer_manager.h"

timer_manager* timer_manager::ins_ = nullptr;
uint64_t timer_manager::timer_seq_ = 0;

timer_manager::timer_manager(boost::asio::io_context &io) : io_(io) {
    ins_ = this;
}

timer_manager::~timer_manager() {
    clear();
}

uint64_t timer_manager::add_timer(uint32_t sec, TIMEOUT_HANDLE f) {
    boost::unique_lock<boost::shared_mutex> ul(sm_);
    uint64_t timer_id = get_id();
    timers_[timer_id] = std::make_shared<timer_actor>(io_, sec, f);
    timers_[timer_id]->start();

    return timer_id;
}

void timer_manager::del_timer(uint64_t timer_id) {
    boost::unique_lock<boost::shared_mutex> ul(sm_);
    //timers_.erase(timer_id);
    auto it = timers_.find(timer_id);
    if (it != timers_.end()) {
        it->second->cancel();
        timers_.erase(it);
    }
}

void timer_manager::clear() {
    timers_.clear();
}
