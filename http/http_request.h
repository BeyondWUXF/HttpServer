//
// Created by wuxiaofeng on 2019/5/28.
//
#pragma once

#include "../util/vendor_boost.h"

class http_request {
public:
    http_request(const boost::beast::http::request<boost::beast::http::string_body> &req);

    inline unsigned version() const noexcept {
        return req_.version();
    }

    // see: boost/beast/http/verb.hpp
    inline boost::beast::http::verb method() const {
        return req_.method();
    }

    inline std::string method_string() const {
        return req_.method_string().to_string();
    }

    // eg: /uri/test?param=1&param=2
    inline std::string target() const {
        return req_.target().to_string();
    }

    inline bool chunked() const {
        return req_.chunked();
    }

    inline bool has_content_length() const {
        return req_.has_content_length();
    }

    inline bool keep_alive() const {
        return req_.keep_alive();
    }

    inline bool need_eof() const {
        return req_.need_eof();
    }

    inline std::string header(const std::string &key) const {
        return req_.base()[key].to_string();
    }

    inline std::string path() const {
        return path_;

    }

    inline bool has_param() const {
        return !params_.empty();
    }

    inline std::string param(const std::string &name) {
        return params_[name];
    }

    inline std::string body() const{
        return req_.body();
    }

private:
    void parse_param();
    void split_params(const std::string &str, const std::string &sep, std::map<std::string, std::string> &param);
    std::string url_decode(const std::string& str);

private:
    std::string path_;
    std::map<std::string, std::string> params_;

    boost::beast::http::request<boost::beast::http::string_body> req_;
};
