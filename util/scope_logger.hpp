//
// Created by wuxiaofeng on 2019/5/31.
//
#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <chrono>

#include "vendor_boost.h"

class default_warn_writer {
public:
    static void write(const std::string &s) {
        //std::cout << s << std::endl;
        BOOST_LOG_TRIVIAL(warning) << s;
    }
};

class default_info_writer {
public:
    static void write(const std::string &s) {
        BOOST_LOG_TRIVIAL(info) << s;
    }
};

template <typename warn_writer = default_warn_writer, typename info_writer = default_info_writer>
class scope_logger {
public:
    scope_logger(const std::string &name, int warn_delay = 50)
    : name_(name), warn_delay_(warn_delay), begin_time_(std::chrono::system_clock::now()) {
        ++query_count_[name_];
    }

    ~scope_logger() {
        auto during = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin_time_);
        if (during.count() < warn_delay_) {
            std::stringstream s;
            s << name_ << " takes " << during.count() << " ms. Total count: " << query_count_[name_]
                        << ", slow count: " << slow_count_[name_];
            info_writer::write(s.str());
        } else {
            ++slow_count_[name_];
            std::stringstream s;
            s << name_ << " takes " << during.count() << " ms. Total count: " << query_count_[name_]
                        << ", slow count: " << slow_count_[name_];
            warn_writer::write(s.str());
        }
    }

    void set_warn_delay(int value) {
        warn_delay_ = value;
    }

private:
    int warn_delay_;
    std::string name_;
    std::chrono::system_clock::time_point begin_time_;

    static std::map<std::string, uint64_t> query_count_;
    static std::map<std::string, uint64_t> slow_count_;
};

template <typename warn_writer, typename info_writer>
std::map<std::string, uint64_t> scope_logger<warn_writer, info_writer>::query_count_;

template <typename warn_writer, typename info_writer>
std::map<std::string, uint64_t> scope_logger<warn_writer, info_writer>::slow_count_;
