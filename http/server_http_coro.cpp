//
// Created by wuxiaofeng on 2019/5/27.
//

#include "server_http_coro.h"
#include "session_http_coro.h"
#include "../src/config.h"

server_http_coro::server_http_coro(boost::asio::io_context &io, int port, int listen) : acceptor_(io) {
    boost::system::error_code ec;
    try {
        boost::asio::ip::tcp::endpoint addr(boost::asio::ip::tcp::v6(), port);
        acceptor_.open(addr.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Set reuse address error: " << ec.message();
            exit(1);
        }
        acceptor_.bind(addr);
        acceptor_.listen(listen);
    } catch (std::exception &ec) {
        BOOST_LOG_TRIVIAL(error) << "Init server listen at port " << port << " fail! " << ec.what();
        exit(1);
    }
}

void server_http_coro::run(boost::asio::yield_context yield) {
    boost::system::error_code ec;
    while (true) {
        auto session = std::make_shared<session_http_coro>(acceptor_.get_executor().context(), this);
        acceptor_.async_accept(session->socket_, yield[ec]);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Accep error: " << ec.message();
            continue;
        }
        BOOST_LOG_TRIVIAL(debug) << "Accep request " << session->socket_.remote_endpoint(ec);
        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "remote_endpoint http error: " << ec.message();
            continue;
        }
        // 在单独的协程当中运行 session
        boost::asio::spawn(acceptor_.get_executor(), std::bind(&session_http_coro::run, session, std::placeholders::_1));
    }
}

void server_http_coro::handle_func(const std::string &uri, URI_HANDLE f) {
    handle_func_[uri] = f;
}


