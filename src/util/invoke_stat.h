//
// Created by wuxiaofeng on 2019/6/3.
//

#ifndef HTTPSERVER_INVOKE_STAT_H
#define HTTPSERVER_INVOKE_STAT_H

#include <iostream>
#include <map>
#include <chrono>

#include "vendor_boost.h"

#define MAX_SECONDS     60

class invoke_stat {
public:
    invoke_stat();
    uint64_t add(const std::string &name);

    static invoke_stat* get() {return ins_;};
    static void dump();

public:
    class stat {
    public:
        uint64_t add();
        std::map<time_t, uint64_t> get_counts();
        friend std::ostream& operator<<(std::ostream& out, const stat &s);

    private:
        boost::shared_mutex sm_;
        std::map<time_t, std::atomic<uint64_t> > count_;    // 以秒为单位，保存最近一分钟
    };

private:
    static invoke_stat *ins_;
    static boost::shared_mutex sm_;
    static std::unordered_map<std::string, std::shared_ptr<stat> > globle_count_;
};


#endif //HTTPSERVER_INVOKE_STAT_H
