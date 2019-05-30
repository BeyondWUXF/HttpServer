#include <iostream>
#include <thread>
#include "config.h"
#include "../http/coro/server_http_coro.h"
#include "../http/async/server_http_async.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

boost::asio::io_context io;

void handle_test(http_request &req, http_response &res) {
    int i = std::stol(const_cast<http_request &>(req).param("aaaa"));
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body(std::string("{\"uri\":\"" + req.path() + "\", \"param\":\"" + req.param("aaaa") + "\", \"appid\":\"" + req.header("appid") + "\"}"));
}

void handle_chunked(http_request &req, http_response &res) {
    res.chunked(true);
    res.set(boost::beast::http::field::content_type, "text/text");
    res.body("fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n"
             "fjdkslajiodsajfdjsak;fjdksoavdiisoapjfdowpqmfkdlasjvdopsajfkdlsajvkcls;ajfodpajfidopanvkdjsaiiofpjdisaopjvidsopajfidspajfvc\r\n");
}

void handle_rapidjson(http_request &req, http_response &res) {
    rapidjson::Document doc;
    doc.Parse(req.body().c_str());
    if (!doc.IsObject()) {
        res.result(boost::beast::http::status::bad_request);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body("Illegal json");
        return;
    }

    rapidjson::Document docRes;
    rapidjson::Value v;
    docRes.SetObject();

    docRes.AddMember("respon", "respon from server", docRes.GetAllocator());
    docRes.AddMember("param", rapidjson::Value(req.param("aaaa").c_str(), docRes.GetAllocator()).Move(), docRes.GetAllocator());
    docRes.AddMember("appid", rapidjson::Value(req.header("appid").c_str(), docRes.GetAllocator()).Move(), docRes.GetAllocator());
    docRes.AddMember("paString", doc["paString"], docRes.GetAllocator());
    docRes.AddMember("paNumber", doc["paNumber"], docRes.GetAllocator());
    docRes.AddMember("paBool", doc["paBool"], docRes.GetAllocator());
    docRes.AddMember("paNull", doc["paNull"], docRes.GetAllocator());
    docRes.AddMember("paDouble", doc["paDouble"], docRes.GetAllocator());
    docRes.AddMember("paArr", doc["paArr"], docRes.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    docRes.Accept(writer);

    res.set(boost::beast::http::field::content_type, "application/json");
    res.body(buffer.GetString());
}

int main(int argc, char *argv[]) {
    // 系统配置初始化
    config config_(io, argc, argv);

    // http服务协程
    server_http_coro http(io, config::get()->port, config::get()->listen);
    http.handle_func(std::string("/IrcChatData/Test"), handle_test);
    http.handle_func(std::string("/IrcChatData/Chunk"), handle_chunked);
    http.handle_func(std::string("/IrcChatData/rapidjson"), handle_rapidjson);

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
