//
// Created by wuxiaofeng on 2019/6/4.
//

#include "timer_actor.h"


timer_actor::timer_actor(boost::asio::io_context &io, uint32_t sec, TIMEOUT_HANDLE func) :
    timer_(io, boost::posix_time::seconds(sec)), handle_(func), sec_(sec) {
}

timer_actor::~timer_actor() {
    cancel();
    //BOOST_LOG_TRIVIAL(trace) << "~timer_actor";
}

void timer_actor::start() {
    timer_.async_wait(boost::bind(&timer_actor::on_timer, this->shared_from_this(), boost::asio::placeholders::error));
}

void timer_actor::handle(TIMEOUT_HANDLE func) {
    handle_ = func;
}

void timer_actor::renew(uint32_t sec) {
    timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(sec));
    timer_.async_wait(boost::bind(&timer_actor::on_timer, this->shared_from_this(), boost::asio::placeholders::error));
}

void timer_actor::cancel() {
    boost::system::error_code ec;
    timer_.cancel(ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Timer cancel failed! " <<  ec.message();
    }
}

void timer_actor::on_timer(const boost::system::error_code &ec) {
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << ec.message() << ".timer is canceled!";
        return;
    }

    if (handle_()) {
        renew(sec_);
    }
}

