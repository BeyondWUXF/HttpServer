//
// Created by wuxiaofeng on 2019/5/27.
//
#pragma once

#include "../util/vendor_boost.h"
#include "http_request.h"
#include "http_response.h"

// return: true-has next packet(trunk), false-last packet
using URI_HANDLE =  std::function<void (http_request &, http_response &)>;

class server_http_coro {
public:
    server_http_coro(boost::asio::io_context &io, int port, int listen);
    void handle_func(const std::string &uri, URI_HANDLE f);
    void run(boost::asio::yield_context);

private:
    boost::asio::ip::tcp::acceptor acceptor_;

    std::map<std::string, URI_HANDLE > handle_func_;
    friend class session_http_coro;
};
