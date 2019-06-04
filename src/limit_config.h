//
// Created by wuxiaofeng on 2019/6/3.
//
#pragma once

#include <iostream>

#include "vendor_boost.h"

#define LIMIT_INFINITE  99999999

class limit_config {
public:
    limit_config(boost::asio::io_context &io, const std::string &exe);

    static limit_config* get() {return ins_;}

    uint64_t get_limit(const std::string &s);

    void load_config();

private:
    void reload(const boost::system::error_code &ec, int signal_numer);

private:
    static limit_config *ins_;

    boost::shared_mutex sm_;
    std::unordered_map<std::string, uint64_t> limits_;

    boost::filesystem::path root_;
    boost::asio::signal_set     log_sigs_;
};
