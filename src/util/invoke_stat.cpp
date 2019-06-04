//
// Created by wuxiaofeng on 2019/6/3.
//

#include "invoke_stat.h"

invoke_stat* invoke_stat::ins_ = nullptr;
boost::shared_mutex invoke_stat::sm_;
std::unordered_map<std::string, std::shared_ptr<invoke_stat::stat> > invoke_stat::globle_count_;

invoke_stat::invoke_stat() {
    ins_ = this;
}

uint64_t invoke_stat::add(const std::string &name) {
    boost::shared_lock<boost::shared_mutex> sl(sm_);

    if (globle_count_.find(name) == globle_count_.end()) {
        sl.unlock();
        boost::unique_lock<boost::shared_mutex> ul(sm_);
        if (globle_count_.find(name) == globle_count_.end()) {  // 防止其它线程已经创建
            globle_count_[name] = std::make_shared<invoke_stat::stat>();
        }
    }

    return globle_count_[name]->add();
}

uint64_t invoke_stat::stat::add() {
    boost::shared_lock<boost::shared_mutex> sl(sm_);

    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (count_.find(now) == count_.end()) {
        sl.unlock();
        boost::unique_lock<boost::shared_mutex> ul(sm_);
        ++count_[now];
        for (auto it = count_.begin(); it != count_.end();) {
            if (now - it->first > MAX_SECONDS) {
                it = count_.erase(it);
            } else {
                ++it;
                break;
            }
        }
    } else {
        ++count_[now];
    }

    return count_[now].load();
}

std::map<time_t, uint64_t> invoke_stat::stat::get_counts() {
    std::map<time_t, uint64_t> result;
    boost::shared_lock<boost::shared_mutex> sl(sm_);
    for (auto &it : count_) {
        result[it.first] = it.second.load();
    }

    return std::move(result);
}

std::ostream& operator<<(std::ostream& out, const invoke_stat::stat &s) {
    std::map<time_t, std::atomic<uint64_t> > count;

    do {
        boost::shared_lock<boost::shared_mutex> sl(const_cast<invoke_stat::stat &>(s).sm_);
        for (auto &it : s.count_) {
            count[it.first] = it.second.load();
        }
    } while(false);

    for (auto &it : count) {
        time_t t = it.first;
        char buf[32] = {0};
        std::strftime(buf, sizeof(buf), "\t\t[%F %T]", std::localtime(&t));
        out << buf << " invoke " << it.second << " times.\n";
        //std::put_time(std::localtime(&t), "[%F %T]") g++4.8.2 #include <iomanip>里没有这个函数
    }
    return out;
}

void invoke_stat::dump() {
    std::unordered_map<std::string, std::shared_ptr<stat> > globle_count;

    do {
        boost::shared_lock<boost::shared_mutex> sl(sm_);
        globle_count.clear();
        //std::copy(globle_count_.begin(), globle_count_.end(), globle_count.begin());
        for (auto &it : globle_count_) {
            globle_count[it.first] = it.second;
        }
    } while (false);

    BOOST_LOG_TRIVIAL(info) << "----------------------dump invoke stat----------------------";
    for (auto &it : globle_count) {
        BOOST_LOG_TRIVIAL(info) << "\n\t[INVOKE_DUMP][" << it.first << "] \n" << *(it.second);
    }
}
