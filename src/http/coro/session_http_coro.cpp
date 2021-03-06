//
// Created by wuxiaofeng on 2019/5/27.
//

#include "session_http_coro.h"
#include "server_http_coro.h"
#include "invoke_stat.h"
#include "limit_check.h"

session_http_coro::session_http_coro(boost::asio::io_context &io, server_http_coro *server) : socket_(io), server_(server) {
    //socket_.
}

void session_http_coro::send_response(boost::asio::yield_context yield, const boost::beast::http::response<boost::beast::http::string_body> &res) {
    boost::system::error_code ec;

    BOOST_LOG_TRIVIAL(debug) << "Write: " << res;
    BOOST_LOG_TRIVIAL(info) << "Write: " << res.result_int() << " " << res.body();
    boost::beast::http::async_write(socket_, res, yield[ec]);
    if(ec) {
        BOOST_LOG_TRIVIAL(error) << "Write fail: " << ec.message();
    }
}

void session_http_coro::handle_request(boost::asio::yield_context yield) {
    boost::system::error_code ec;
    http_request req(req_);
    http_response res(200, req_.version());
    res.keep_alive(req_.keep_alive());

    if (server_->handle_func_.find(req.path()) != server_->handle_func_.end()) {
        if (limit_check::check(req.path(), invoke_stat::get()->add(req.path()))) {
            res.result(boost::beast::http::status::service_unavailable);
            res.set(boost::beast::http::field::content_type, "text/plain");
            res.body("Out of limit!");
        } else {
            server_->handle_func_[req.path()](req, res, socket_.get_io_context(), yield);
        }
    } else {
        res.result(boost::beast::http::status::not_found);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body(req.path() + " not found!");
    }

    send_response(yield, res.res_);
}

void session_http_coro::run(boost::asio::yield_context yield) {
    boost::system::error_code ec;
    boost::beast::flat_buffer buffer;

    try {
        while (true) {
            boost::beast::http::async_read(socket_, buffer, req_, yield[ec]);
            if (ec) {
                BOOST_LOG_TRIVIAL(warning) << "Read fail: " << ec.message();
                break;
            }
            BOOST_LOG_TRIVIAL(debug) << "Read: " << req_;
            BOOST_LOG_TRIVIAL(info) << "Read: " << req_.method_string() << " " << req_.target() << " " << req_.body();

            handle_request(yield);
            if(req_.need_eof())	{// ture means Connection: close
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                break;
            }
            req_.body().clear();
        }
        // Send a TCP shutdown
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    } catch (std::exception &ec) {
        BOOST_LOG_TRIVIAL(error) << ec.what();
    }
}
