//
// Created by wuxiaofeng on 2019/6/4.
//

#ifndef HTTPSERVER_LIMIT_CHECK_H
#define HTTPSERVER_LIMIT_CHECK_H

#include <iostream>
#include "../limit_config.h"
#include "../config.h"

class limit_check {
public:
    static bool check(const std::string &name, uint64_t cur_count) {
        if (config::get()->limit) {
            return cur_count > limit_config::get()->get_limit(name);
        } else {
            return false;
        }
    };

private:
};


#endif //HTTPSERVER_LIMIT_CHECK_H
