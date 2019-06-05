//
// Created by wuxiaofeng on 2019/6/4.
//

#include "string_util.h"

std::vector<std::string> string_util::split_sep(const std::string &src, const std::string &seps) {
    std::vector<std::string> res;
    size_t begin = 0, end = std::string::npos, equal_sep = std::string::npos;
    std::string one;
    do {
        end = src.find_first_of(seps, begin);
        res.emplace_back(src.substr(begin, end - begin));
        begin = end + 1;
    } while (end != std::string::npos);

    return std::move(res);
}
