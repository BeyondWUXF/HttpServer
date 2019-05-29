//
// Created by wuxiaofeng on 2019/5/27.
//

#include "server_http_async.h"
#include "session_http_async.h"
#include "../src/config.h"

server_http_async::server_http_async(boost::asio::io_context &io) : acceptor_(io), socket_(io) {
    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint addr(boost::asio::ip::tcp::v6(), config::get()->port);
    acceptor_.open(addr.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    acceptor_.bind(addr);
    acceptor_.listen(config::get()->listen);
}

void server_http_async::run() {
    do_accept();
}

void server_http_async::do_accept() {
    acceptor_.async_accept(
            socket_,
            std::bind(
                    &server_http_async::on_accept,
                    shared_from_this(),
                    std::placeholders::_1));
}

void server_http_async::on_accept(boost::system::error_code ec) {
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Accep error: " << ec.message();
    } else {
        std::make_shared<session_http_async>(acceptor_.get_executor().context(), std::move(socket_))->run();
    }
    do_accept();
}
