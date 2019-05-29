//
// Created by wuxiaofeng on 2019/5/27.
//
#pragma once

#include <memory>

#include "vendor_boost.h"

class server_http_async : public std::enable_shared_from_this<server_http_async> {
public:
    server_http_async(boost::asio::io_context &io);
    void run();

private:
    void do_accept();
    void on_accept(boost::system::error_code ec);

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
};
