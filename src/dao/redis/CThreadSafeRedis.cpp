//
// Created by wuxiaofeng on 2019/6/5.
//

#include "CThreadSafeRedis.h"
#include <time.h>
#include <sys/syscall.h>

CThreadSafeRedis* CThreadSafeRedis::ptr_;

pid_t getCurThreadId()
{
    return syscall(SYS_gettid);
}

#include<pthread.h>
__thread CRedisDBInterface* __redis = nullptr;

pthread_key_t gThreadRedisKey;

void deleteRedisObj(void * obj) {
    assert(obj == __redis);
    delete __redis;
    __redis = nullptr;
}

CRedisDBInterface* CThreadSafeRedis::getRedis(const std::string& host, int port, const std::string &pwd) {
    if (__redis != nullptr) {
        return __redis;
    }

    ///not find, create one.
    __redis = new(std::nothrow) CRedisDBInterface();
    if (__redis == nullptr) {
        BOOST_LOG_TRIVIAL(error) << "No memory to new RedisDbInterface.";
        return nullptr;
    }

    if (pthread_key_create(&gThreadRedisKey, deleteRedisObj) != 0) {
        BOOST_LOG_TRIVIAL(warning) << "pthread_key_create is failed";
    }
    if (pthread_setspecific(gThreadRedisKey, __redis) != 0) {
        BOOST_LOG_TRIVIAL(warning) << "pthread_setspecific is failed";
    }

    __redis->setAuthPassword(pwd);
    if (!__redis->ConnectDB(host, port))
    {
        BOOST_LOG_TRIVIAL(error) << "InitRedis failed for thread: " << getCurThreadId();
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "InitRedis successed for thread: " << getCurThreadId() << " to " << host << ":" << port;
    }

    return __redis;
}
