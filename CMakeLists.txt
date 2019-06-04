cmake_minimum_required(VERSION 3.13)
project(HttpServer)

set(CMAKE_CXX_STANDARD 11)
set(BOOST_INC_DIR /usr/local/boost/include)
set(BOOST_LIB_DIR /usr/local/boost/lib)
#set(BOOST_LIB)

include_directories(${BOOST_INC_DIR} vendor vendor/hiredis/include util http)
link_directories(${BOOST_LIB_DIR} vendor/hiredis/lib)
#link_libraries(boost_coroutine boost_context boost_filesystem boost_program_options boost_log_setup boost_log boost_system boost_thread boost_regex pthread rt dl z)

add_executable(HttpServer src/http_main.cpp util/url.cpp util/url.h util/local_addr.cpp util/local_addr.h src/config.cpp src/config.h http/coro/session_http_coro.cpp http/coro/session_http_coro.h util/vendor_boost.h http/coro/server_http_coro.cpp http/coro/server_http_coro.h http/async/session_http_async.cpp http/async/session_http_async.h http/async/server_http_async.cpp http/async/server_http_async.h http/http_request.cpp http/http_request.h http/http_response.cpp http/http_response.h util/scope_logger.hpp util/invoke_stat.cpp util/invoke_stat.h src/limit_config.cpp src/limit_config.h util/limit_check.cpp util/limit_check.h util/timer_actor.h util/timer_actor.cpp util/timer_manager.cpp util/timer_manager.h)

target_link_libraries(HttpServer -lhiredis -Wl,-Bstatic -lboost_coroutine -lboost_context -lboost_filesystem -lboost_program_options -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lboost_regex -Wl,-Bdynamic -lpthread -lrt -ldl -lz)