//
// Created by wuxiaofeng on 2019/5/24.
//
#pragma once

#include <iostream>
#include <string>

#include "../util/vendor_boost.h"
#include "../util/local_addr.h"

class config : public boost::noncopyable {
public:
    config(boost::asio::io_context &io, int argc, char *argv[]);
    inline static config* get() { return ins_;}

    std::string local_ip;
    int worker;
    int listen;
    int port;
    struct {
        std::string host;
        int port;
    }pika;

private:
    static config *ins_;

    void init_options(int argc, char *argv[]);
    void init_logging(boost::asio::io_context &io);
    void rotate_log(const boost::system::error_code &error, int signal_numer);

    std::string environ_;
    std::string log_path_;
    boost::filesystem::path root_;
    boost::log::trivial::severity_level log_level_;

    using LogBackT = boost::log::sinks::text_file_backend;
    typedef boost::log::sinks::asynchronous_sink<LogBackT> LogSinkT;
    // 注意, 这里需要使用 boost 提供的 shared_ptr (log 库内部使用)
    boost::shared_ptr<LogBackT>  log_back_;
    boost::asio::signal_set     log_sigs_;
    boost::log::sources::logger lg_;
};
