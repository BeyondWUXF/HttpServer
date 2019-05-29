cmake_minimum_required(VERSION 3.13)
project(HttpServer)

set(CMAKE_CXX_STANDARD 11)
set(BOOST_INC_DIR /usr/local/boost/include)
set(BOOST_LIB_DIR /usr/local/boost/lib)
#set(BOOST_LIB)

include_directories(${BOOST_INC_DIR})
link_directories(${BOOST_LIB_DIR})
#link_libraries(boost_coroutine boost_context boost_filesystem boost_program_options boost_log_setup boost_log boost_system boost_thread boost_regex pthread rt dl z)

add_executable(HttpServer src/main.cpp util/url.cpp util/url.h util/local_addr.cpp util/local_addr.h src/config.cpp src/config.h http/session_http_coro.cpp http/session_http_coro.h util/vendor_boost.h http/server_http_coro.cpp http/server_http_coro.h http/session_http_async.cpp http/session_http_async.h http/server_http_async.cpp http/server_http_async.h http/http_request.cpp http/http_request.h http/http_response.cpp http/http_response.h)

target_link_libraries(HttpServer -Wl,-Bstatic -lboost_coroutine -lboost_context -lboost_filesystem -lboost_program_options -lboost_log_setup -lboost_log -lboost_system -lboost_thread -lboost_regex -Wl,-Bdynamic -lpthread -lrt -ldl -lz)