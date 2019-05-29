//
// Created by wuxiaofeng on 2019/5/27.
//
#pragma once

#include <memory>

#include "../util/vendor_boost.h"

class session_http_async : public std::enable_shared_from_this<session_http_async> {
public:
    session_http_async(boost::asio::io_context &io, boost::asio::ip::tcp::socket socket);
    void run();

private:
    void handle_request();
    void send_response(const std::string &body);

    void do_read();
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
    void on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close);
    void do_close();

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::flat_buffer buffer_;
    std::shared_ptr<void> res_;

    friend class server_http_async;
};
