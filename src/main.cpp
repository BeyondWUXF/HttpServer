#include <iostream>
#include <thread>
#include "config.h"
#include "../http/server_http_coro.h"
#include "../http/server_http_async.h"

boost::asio::io_context io;

void handle_test(http_request &req, http_response &res) {
    int i = std::stol(const_cast<http_request &>(req).param("aaaa"));
    res.set(boost::beast::http::field::content_type, "text/json");
    res.body(std::string("{\"uri\":\"" + req.path() + "\", \"param\":\"" + req.param("aaaa") + "\", \"appid\":\"" + req.header("appid") + "\"}"));
}

void handle_chunked(http_request &req, http_response &res) {
    res.chunked(true);
    res.set(boost::beast::http::field::content_type, "text/text");
    res.body("fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n"
             "fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n");
}

int main(int argc, char *argv[]) {
    // 系统配置初始化
    config config_(io, argc, argv);

    // http服务协程
    server_http_coro http(io, config::get()->port, config::get()->listen);
    http.handle_func(std::string("/IrcChatData/Test"), handle_test);
    http.handle_func(std::string("/IrcChatData/Chunk"), handle_chunked);

    boost::asio::spawn(io, std::bind(&server_http_coro::run, &http, std::placeholders::_1));

    std::vector<std::thread> v;
    for (auto i = config::get()->worker; i > 0; i--) {
        v.emplace_back(
            [&]
            {
                io.run();
            });
    }

    // http服务异步
    /*std::make_shared<server_http_async>(io)->run();

    std::vector<std::thread> v;
    for(auto i = config::get()->worker; i > 0; --i) {
        v.emplace_back([&] {run(std::ref(io));});
    }*/

    io.run();
    return 0;
}
