cmake_minimum_required(VERSION 3.13)
project(HttpServer)

set(CMAKE_CXX_STANDARD 11)
set(BOOST_INC_DIR /usr/local/boost/include)
set(BOOST_LIB_DIR /usr/local/boost/lib)
#set(BOOST_LIB)

include_directories(${BOOST_INC_DIR} src/vendor src/vendor/hiredis/include src/util src/http src/dao/redis)
link_directories(${BOOST_LIB_DIR} src/vendor/hiredis/lib)
#link_libraries(boost_coroutine boost_context boost_filesystem boost_program_options boost_log_setup boost_log boost_system boost_thread boost_regex pthread rt dl z)

add_executable(HttpServer src/http_main.cpp src/util/url.cpp src/util/url.h src/util/local_addr.cpp src/util/local_addr.h src/config.cpp src/config.h src/http/coro/session_http_coro.cpp src/http/coro/session_http_coro.h src/util/vendor_boost.h src/http/coro/server_http_coro.cpp src/http/coro/server_http_coro.h src/http/async/session_http_async.cpp src/http/async/session_http_async.h src/http/async/server_http_async.cpp src/http/async/server_http_async.h src/http/http_request.cpp src/http/http_request.h src/http/http_response.cpp src/http/http_response.h src/util/scope_logger.hpp src/util/invoke_stat.cpp src/util/invoke_stat.h src/limit_config.cpp src/limit_config.h src/util/limit_check.cpp src/util/limit_check.h src/util/timer_actor.h src/util/timer_actor.cpp src/util/timer_manager.cpp src/util/timer_manager.h src/util/string_util.cpp src/util/string_util.h src/dao/redis/CDBInterface.h src/dao/redis/CRedisDBInterface.cpp src/dao/redis/CRedisDBInterface.h src/dao/redis/CThreadSafeRedis.cpp src/dao/redis/CThreadSafeRedis.h)

target_link_libraries(HttpServer -lhiredis -Wl,-Bstatic -lboost_coroutine -lboost_context -lboost_filesystem -lboost_program_options -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lboost_regex -Wl,-Bdynamic -lpthread -lrt -ldl -lz)