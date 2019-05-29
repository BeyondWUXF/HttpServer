//
// Created by wuxiaofeng on 2019/5/29.
//
#pragma once

#include "vendor_boost.h"

class http_response {
public:
    http_response(unsigned status, unsigned version);

    inline void version(unsigned value) noexcept {
        res_.version(value);
    }

    // see: boost/beast/http/status.hpp
    inline void result(boost::beast::http::status v) {
        res_.result(v);
    }

    inline void result(unsigned v) {
        // see: boost/beast/http/status.hpp
        res_.result(boost::beast::http::int_to_status(v));
    }

    inline void reason(std::string v) {
        res_.reason(v);
    }

    inline void chunked(bool v) {
        res_.chunked(v);
    }

    inline size_t content_length() {
        return res_.body().length();
    }

    inline void content_length(size_t v) {
        res_.content_length(v);
    }

    inline void keep_alive(bool v) {
        res_.keep_alive(v);
    }

    inline void body(const std::string &v) {
        if (!res_.chunked()) {
            content_length(v.length());
        }
        res_.body() = v;
    }

    // see boost/beast/http/field.hpp
    inline void set(boost::beast::http::field f, const std::string &v) {
        res_.set(f, v);
    }

    inline void set(const std::string &f, const std::string &v) {
        res_.set(f, v);
    }

private:
    boost::beast::http::response<boost::beast::http::string_body> res_;

    friend class session_http_coro;
};
