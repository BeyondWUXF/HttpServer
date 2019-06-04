//
// Created by wuxiaofeng on 2019/6/3.
//

#include "limit_config.h"

limit_config* limit_config::ins_ = nullptr;

limit_config::limit_config(boost::asio::io_context &io, const std::string &exe) : log_sigs_(io) {
    root_ = boost::filesystem::absolute(exe).lexically_normal().parent_path().parent_path();
    load_config();
    ins_ = this;
    log_sigs_.add(SIGUSR1);
}

uint64_t limit_config::get_limit(const std::string &s) {
    boost::shared_lock<boost::shared_mutex> sl(sm_);
    auto it = limits_.find(s);
    if (it == limits_.end()) {
        return LIMIT_INFINITE;
    } else {
        return it->second;
    }
}

void limit_config::load_config() {
    // 配置文件路径按不同环境区分加载
    boost::filesystem::path cfile = root_.native() + std::string("/etc/Limit") + std::string(".ini");
    std::unordered_map<std::string, uint64_t> temp;

    try {
        boost::property_tree::ptree m_pt, tag, attr;
        boost::property_tree::read_ini(cfile.string(), m_pt);

        temp.clear();
        BOOST_FOREACH(boost::property_tree::ptree::value_type &tag, m_pt) {
            //std::cout << tag.first << "=" << tag.second.data() << std::endl;
            BOOST_FOREACH(boost::property_tree::ptree::value_type &attr, tag.second) {
                //std::cout << attr.first << "=" << attr.second.data() << std::endl;
                if (attr.first == "root") {
                    temp[tag.first] = std::atoll(attr.second.data().c_str());
                } else {
                    temp[tag.first + "?" + attr.first] = std::atoll(attr.second.data().c_str());
                }
            }
        }

        // 没有异常
        if (!temp.empty()) {
            boost::unique_lock<boost::shared_mutex> ul(sm_);
            std::swap(limits_, temp);
        }

        for (auto &it : limits_) {
            BOOST_LOG_TRIVIAL(info) << "[LIMIT_CONFIG] " << it.first << "\t\t\t\t max qps: " << it.second;
        }
    } catch (const std::exception &ec) {
        BOOST_LOG_TRIVIAL(error) << "Load limit config fail! " << ec.what();
    }

    // 信号处理进行重载
    log_sigs_.async_wait(std::bind(&limit_config::reload, this, std::placeholders::_1, std::placeholders::_2));
}

void limit_config::reload(const boost::system::error_code &ec, int signal_numer) {
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Reload limit_config Signal error:" << ec.message();
    } else {
        BOOST_LOG_TRIVIAL(warning) << "Reloading limit config......";
        load_config();
        BOOST_LOG_TRIVIAL(warning) << "Reload limit config finish.";
    }
}
