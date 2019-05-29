//
// Created by wuxiaofeng on 2019/5/28.
//

#include "http_request.h"

http_request::http_request(const boost::beast::http::request<boost::beast::http::string_body> &req) : req_(req) {
    parse_param();
}

void http_request::parse_param() {
    params_.clear();
    std::string params;
    size_t begin = req_.target().find("?");
    if (begin == boost::string_view::npos)
    {
        path_ = url_decode(req_.target().to_string());
        return;
    }

    path_ = url_decode(req_.target().substr(0, begin).to_string());
    params = url_decode(req_.target().substr(begin + 1).to_string());
    BOOST_LOG_TRIVIAL(trace) << "Path: " << path_;
    BOOST_LOG_TRIVIAL(trace) << "Params: " << params;
    // 效率不高，tps差5、6百
//	boost::tokenizer<boost::char_separator<char> > tok(params, boost::char_separator<char>("&"));
//	for (auto it = tok.begin(); it != tok.end(); ++it) {
//		boost::tokenizer<boost::char_separator<char> > tokOne(*it, boost::char_separator<char>("="));
//		auto key = tokOne.begin();
//		auto value = key;
//		if (++value != tokOne.end())
//			params_[*key] = *value;
//		else
//			params_[*key] = "";
//	}
    split_params(params, "&", params_);
    for (auto it = params_.begin(); it != params_.end(); ++it) {
        BOOST_LOG_TRIVIAL(trace) << it->first << "=" << it->second;
    }
}

void http_request::split_params(const std::string &str, const std::string &sep, std::map<std::string, std::string> &param) {
    size_t begin = 0, end = std::string::npos, equal_sep = std::string::npos;
    std::string one;
    do {
        end = str.find_first_of(sep, begin);
        one.assign(str, begin, end - begin);
        begin = end + 1;
        boost::algorithm::trim(one);

        if ((equal_sep = one.find("=")) == std::string::npos) {
            param[one] = "";
        } else {
            param[one.substr(0, equal_sep)] = one.substr(equal_sep + 1);
        }
    } while (end != std::string::npos);

    // 效率不高
//	std::vector<std::string> all;
//	size_t equal_sep = std::string::npos;
//	boost::algorithm::split(all, str, boost::is_any_of(sep));
//	for (auto it = all.begin(); it != all.end(); ++it) {
//		if ((equal_sep = it->find("=")) == std::string::npos) {
//			param[*it] = "";
//		} else {
//			param[it->substr(0, equal_sep)] = it->substr(equal_sep + 1);
//		}
//	}
}

std::string http_request::url_decode(const std::string& str)
{
    auto const from_hex = [](unsigned char x) {
        unsigned char y;
        if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
        else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
        else if (x >= '0' && x <= '9') y = x - '0';
        else y = x;
        //else assert(0);
        return y;
    };

    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%') {
            //assert(i + 2 < length);
            if (i + 2 >= length) {
                BOOST_LOG_TRIVIAL(error) << "url_decode error:" << str;
                return "";
            }
            unsigned char high = from_hex((unsigned char)str[++i]);
            unsigned char low = from_hex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
