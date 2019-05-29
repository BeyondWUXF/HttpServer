//
// Created by wuxiaofeng on 2019/5/27.
//

#include "session_http_async.h"

session_http_async::session_http_async(boost::asio::io_context &io, boost::asio::ip::tcp::socket socket)
    : socket_(std::move(socket)), strand_(socket_.get_executor()) {
}

void session_http_async::send_response(const std::string &body) {
    boost::system::error_code ec;
    // Send the response
    auto res = std::make_shared<boost::beast::http::response<boost::beast::http::string_body> >(boost::beast::http::status::ok, req_.version());
    res_ = res;
    res->set(boost::beast::http::field::server, "HttpServerV1.0");
    res->set(boost::beast::http::field::content_type, "text/json");
    res->body() = body;
    res->content_length(body.length());
    res->keep_alive(req_.keep_alive());
    //boost::beast::http::serializer<boost::beast::http::response<boost::beast::http::string_body> > sr{res};
    BOOST_LOG_TRIVIAL(debug) << "Write: " << *res;
    BOOST_LOG_TRIVIAL(info) << "Write: " << res->result_int() << " " << body.length() << " " << res->body();
    boost::beast::http::async_write(
            socket_,
            *res,
            boost::asio::bind_executor(
                    strand_,
                    std::bind(
                            &session_http_async::on_write,
                            shared_from_this(),
                            std::placeholders::_1,
                            std::placeholders::_2,
                            req_.need_eof())));
}

void session_http_async::handle_request() {
    boost::system::error_code ec;

    send_response(std::string("{\"uri\":\"") + req_.target().to_string() + "\"}");
}

void session_http_async::run() {
    do_read();
}

void session_http_async::do_read() {
    boost::beast::http::async_read(socket_, buffer_, req_,
                                   boost::asio::bind_executor(
                                           strand_,
                                           std::bind(
                                                   &session_http_async::on_read,
                                                   shared_from_this(),
                                                   std::placeholders::_1,
                                                   std::placeholders::_2)));
}

void session_http_async::on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == boost::beast::http::error::end_of_stream) {
        return do_close();
    }

    if(ec) {
        BOOST_LOG_TRIVIAL(error) << "Read fail: " << ec.message();
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Read: " << req_;
    BOOST_LOG_TRIVIAL(info) << "Read: " << req_.method_string() << " " << req_.target();

    handle_request();
}

void session_http_async::on_write(boost::system::error_code ec, std::size_t bytes_transferred, bool close) {
    boost::ignore_unused(bytes_transferred);

    if(ec) {
        BOOST_LOG_TRIVIAL(error) << "Write fail: " << ec.message();
    }

    if (close) {
        return do_close();
    }

    res_ = nullptr;
    req_.body().clear();
    do_read();
}

void session_http_async::do_close() {
    boost::system::error_code ec;

    // Send a TCP shutdown
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}

