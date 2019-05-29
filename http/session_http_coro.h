//
// Created by wuxiaofeng on 2019/5/27.
//
#pragma once

#include "../util/vendor_boost.h"
#include "http_request.h"

class server_http_coro;
class session_http_coro {
public:
    session_http_coro(boost::asio::io_context &io, server_http_coro *server);
    void run(boost::asio::yield_context yield);

private:
    void handle_request(boost::asio::yield_context yield);
    void send_response(boost::asio::yield_context yield, const boost::beast::http::response<boost::beast::http::string_body> &res);

private:
    boost::asio::ip::tcp::socket socket_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    server_http_coro *server_;

    friend class server_http_coro;
};
