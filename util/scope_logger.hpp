//
// Created by wuxiaofeng on 2019/5/31.
//
// 调用时间记录
// 线程安全
#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include <chrono>

#include "vendor_boost.h"

class default_scope_warn_recorder {
public:
    static void record(const std::string &s) {
        //std::cout << s << std::endl;
        BOOST_LOG_TRIVIAL(warning) << s;
    }
};

class default_scope_total_recorder {
public:
    static void record(const std::string &s) {
        BOOST_LOG_TRIVIAL(info) << s;
    }
};

class default_scope_dump_recorder {
public:
    static void dump(const std::string &type, const std::unordered_map<std::string, std::atomic<uint64_t> > &count) {
        for (auto &one : count) {
            BOOST_LOG_TRIVIAL(info) << "[SCOPE_DUMP][" << type << "][" << one.first << "] Count: " << one.second;
        }
    }
};

template <typename warn_recorder = default_scope_warn_recorder,
          typename total_recorder = default_scope_total_recorder,
          typename dump_recorder = default_scope_dump_recorder>
class scope_logger {
public:
    scope_logger(const std::string &name, int warn_delay = 50)
    : name_(name), warn_delay_(warn_delay), begin_time_(std::chrono::system_clock::now()) {
        add_query();
    }

    ~scope_logger() {
        auto during = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - begin_time_);
        if (during.count() < warn_delay_) {
            std::stringstream s;
            s << "[SCOPE_RT][" << name_ << "] takes " << during.count() << " ms. Total count: " << query_count_[name_]
                        << ", slow count: " << slow_count_[name_];
            total_recorder::record(s.str());
        } else {
            add_slow();
            std::stringstream s;
            s << "[SCOPE_RT][" << name_ << "] takes " << during.count() << " ms. Total count: " << query_count_[name_]
                        << ", slow count: " << slow_count_[name_];
            warn_recorder::record(s.str());
        }
    }

    void set_warn_delay(int value) {
        warn_delay_ = value;
    }

    static void dump() {
        std::unordered_map<std::string, std::atomic<uint64_t> > query;
        std::unordered_map<std::string, std::atomic<uint64_t> > slow;

        {
            boost::shared_lock<boost::shared_mutex> sl(query_sm_);
            query.clear();
            std::copy(query_count_.begin(), query_count_.end(), query.begin());
        }

        {
            boost::shared_lock<boost::shared_mutex> sl(slow_sm_);
            slow.clear();
            std::copy(slow_count_.begin(), slow_count_.end(), slow.begin());
        }

        default_scope_dump_recorder::dump("TotalInvoke", query);
        default_scope_dump_recorder::dump("SlowInvoke", slow);
    }

private:
    void add_query() {
        boost::shared_lock<boost::shared_mutex> sl(query_sm_);
        if (query_count_.find(name_) == query_count_.end()) {
            sl.unlock();
            boost::unique_lock<boost::shared_mutex> ul(query_sm_);
            ++query_count_[name_];  // 用++而不是直接初始化为1，是为了防止其它线程获得写锁后又重置为1，而实际应该是2
        } else {
            ++query_count_[name_];
        }
    }

    void add_slow() {
        boost::shared_lock<boost::shared_mutex> sl(slow_sm_);
        if (slow_count_.find(name_) == slow_count_.end()) {
            sl.unlock();
            boost::unique_lock<boost::shared_mutex> ul(slow_sm_);
            ++slow_count_[name_];
        } else {
            ++slow_count_[name_];
        }
    }

private:
    int warn_delay_;
    std::string name_;
    std::chrono::system_clock::time_point begin_time_;

    static boost::shared_mutex query_sm_;
    static boost::shared_mutex slow_sm_;
    static std::unordered_map<std::string, std::atomic<uint64_t> > query_count_;
    static std::unordered_map<std::string, std::atomic<uint64_t> > slow_count_;
};

template <typename warn_recorder, typename total_recorder, typename dump_recorder>
boost::shared_mutex scope_logger<warn_recorder, total_recorder, dump_recorder>::query_sm_;

template <typename warn_recorder, typename total_recorder, typename dump_recorder>
boost::shared_mutex scope_logger<warn_recorder, total_recorder, dump_recorder>::slow_sm_;

template <typename warn_recorder, typename total_recorder, typename dump_recorder>
std::unordered_map<std::string, std::atomic<uint64_t> > scope_logger<warn_recorder, total_recorder, dump_recorder>::query_count_;

template <typename warn_recorder, typename total_recorder, typename dump_recorder>
std::unordered_map<std::string, std::atomic<uint64_t> > scope_logger<warn_recorder, total_recorder, dump_recorder>::slow_count_;
