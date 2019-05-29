//
// Created by wuxiaofeng on 2019/5/29.
//

#include "http_response.h"

http_response::http_response(unsigned status, unsigned version)
    : res_(static_cast<boost::beast::http::status>(status), version) {
    res_.set(boost::beast::http::field::server, "HttpServerV1.0");
    //res_.set(boost::beast::http::field::content_type, "text/json");
}
