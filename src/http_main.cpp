#include <iostream>
#include <thread>
#include "config.h"
#include "coro/server_http_coro.h"
#include "async/server_http_async.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "scope_logger.hpp"
#include "hiredis/hiredis.h"


void handle_test(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    int i = std::stol(const_cast<http_request &>(req).param("aaaa"));
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body(std::string("{\"uri\":\"" + req.path() + "\", \"param\":\"" + req.param("aaaa") + "\", \"appid\":\"" + req.header("appid") + "\"}"));
}

void handle_chunked(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    res.chunked(true);
    res.set(boost::beast::http::field::content_type, "text/text");
    res.body("fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n"
             "fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n");
}

void handle_rapidjson(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    rapidjson::Document doc;
    doc.Parse(req.body().c_str());
    if (!doc.IsObject()) {
        res.result(boost::beast::http::status::bad_request);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body("Illegal json");
        return;
    }

    rapidjson::Document docRes(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &alloc = docRes.GetAllocator();
    docRes.AddMember("respon", "respon from server", alloc);
    docRes.AddMember("param", rapidjson::Value(req.param("aaaa").c_str(), docRes.GetAllocator()).Move(), alloc);
    docRes.AddMember("appid", rapidjson::Value(req.header("appid").c_str(), docRes.GetAllocator()).Move(), alloc);
    docRes.AddMember("paString", doc["paString"], alloc);
    docRes.AddMember("paNumber", doc["paNumber"], alloc);
    docRes.AddMember("paBool", doc["paBool"], alloc);
    docRes.AddMember("paNull", doc["paNull"], alloc);
    docRes.AddMember("paDouble", doc["paDouble"], alloc);
    docRes.AddMember("paArr", doc["paArr"], alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    docRes.Accept(writer);

    res.set(boost::beast::http::field::content_type, "application/json");
    res.body(buffer.GetString());
}

void handle_stat(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    scope_logger<> scopeLogger("/IrcChatData/stat", 1);
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body(std::string("{\"uri\":\"" + req.path() + "\", \"param\":\"" + req.param("aaaa") + "\", \"appid\":\"" + req.header("appid") + "\"}"));
    //boost::asio::deadline_timer timer(io, boost::posix_time::seconds(5));
    //boost::system::error_code ec;
    //timer.async_wait(yield[ec]);
    //sleep(5);
    //std::cout << "end:" << ec.message() << std::endl;
}

void handle_redis(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    scope_logger<> scopeLogger("/IrcChatData/redis", 1);
    res.set(boost::beast::http::field::content_type, "application/json");

    redisContext *c = redisConnect(req.param("host").c_str(), std::atoi(req.param("port").c_str()));
    if (c == NULL || c->err) {
        std::string e;
        if (c) {
            e = "Error: %s\n";
            e += c->errstr;
            // handle error
        } else {
            e = "Can't allocate redis context\n";
        }
        res.body(std::string("{\"error\":\"" + e + "\"}"));
        return;
    }
    redisReply *r = (redisReply*)redisCommand(c, "GET %s", "test123");
    std::string s(r->str, r->len);

    freeReplyObject(r);
    redisFree(c);

    res.body(std::string("{\"uri\":\"" + req.path() + "\", \"param\":\"" + req.param("aaaa") + "\", \"appid\":\"" + req.header("appid") +
                                 + "\", \"redis\":\"" + s + "\"}"));

}

void handle_sync(http_request &req, http_response &res, boost::asio::io_context &io, boost::asio::yield_context yield) {
    scope_logger<> scopeLogger("/IrcChatData/sync", 1);
    res.set(boost::beast::http::field::content_type, "application/json");

    //boost::async_result()
}

int main(int argc, char *argv[]) {
    boost::thread_group tg;

    // 系统配置初始化
    boost::asio::io_context io_main;    // 用于信号注册，重新打开日志文件
    config config_(io_main, argc, argv);

    // http服务协程
    boost::asio::io_context io_http;
    server_http_coro http(io_http, config::get()->port, config::get()->listen);
    http.handle_func(std::string("/IrcChatData/Test"), handle_test);
    http.handle_func(std::string("/IrcChatData/Chunk"), handle_chunked);
    http.handle_func(std::string("/IrcChatData/rapidjson"), handle_rapidjson);
    http.handle_func(std::string("/IrcChatData/stat"), handle_stat);
    http.handle_func(std::string("/IrcChatData/redis"), handle_redis);
    http.handle_func(std::string("/IrcChatData/sync"), handle_sync);
    boost::asio::spawn(io_http, std::bind(&server_http_coro::run, &http, std::placeholders::_1));

    // 监听第二个端口，使用另一个contexst，保证一个context被阻塞时，另一个不影响
    /*boost::asio::io_context io_http2;
    server_http_coro http2(io_http2, config::get()->port + 1, config::get()->listen);
    http2.handle_func(std::string("/IrcChatData/rapidjson"), handle_rapidjson);
    http2.handle_func(std::string("/IrcChatData/redis"), handle_redis);
    boost::asio::spawn(io_http2, std::bind(&server_http_coro::run, &http2, std::placeholders::_1));*/

    // 系统配置
    tg.add_thread(new boost::thread( [&] { io_main.run(); }));

    // 第一个http服务
    for (auto i = config::get()->worker; i > 0; i--) {
        tg.add_thread(new boost::thread( [&] { io_http.run(); }));
    }

    // 第二个http服务
    /*for (auto i = config::get()->worker; i > 0; i--) {
        tg.add_thread(new boost::thread( [&] { io_http2.run(); }));
    }*/

    // 其它业务线程
    /*tg.add_thread(new boost::thread(
            []{
                while (true) {
                    std::cout << "Server running......" << std::endl;
                    sleep(5);
                }
            }
            )
    );*/

    tg.join_all();

    // http服务异步
    /*std::make_shared<server_http_async>(io)->run();

    std::vector<std::thread> v;
    for(auto i = config::get()->worker; i > 0; --i) {
        v.emplace_back([&] {run(std::ref(io));});
    }

    io.run();*/
    return 0;
}
