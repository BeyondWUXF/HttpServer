//
// Created by wuxiaofeng on 2019/6/5.
//
#pragma once
#include "CRedisDBInterface.h"

#define g_redis_client (CThreadSafeRedis::get()->getRedis(config::get()->redis.host, config::get()->redis.port, config::get()->redis.pwd))

class CThreadSafeRedis {
public:
    ~CThreadSafeRedis() {};

    static CThreadSafeRedis* get()	{
        return ptr_;
    }

    CRedisDBInterface* getRedis(const std::string& host, int port, const std::string &pwd);

private:
    // 对于一定时间不使用的redis连接，检查其对应的线程是否存在，如果不存在，就收回
    // void checkRedisInstance(pid_t curTid);

private:
    static CThreadSafeRedis*	 ptr_;
};
