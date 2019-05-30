# HttpServer
C++实现基于boost库实现的http服务端

[boost](https://www.boost.org/users/history/version_1_69_0.html)

## 依赖

软件 |软件要求
------|--------
gcc版本:          	|   4.8.5及以上版本、glibc-devel
cmake版本：       	|   3.14.2及以上版本
c++版本：         |	c++11及以上版本
boost版本：         |  1.66.0及以上版本

## 说明

1、使用boost协程库实现并发，经过压力测试协程和异步的QPS没太大差别，选择用协程方式实现更易于代码编写

2、boost1.70.0官方上说有很大改动，理论上使用1.6*.0的版本都能兼容

3、使用boost日志库写日志方式采用异步方式，boost框架单独启一个线程进行写日志

4、使用静态链接方式链接boost库

5、由于网络IO，理论上将工程线程数设置为cpu*2能达到最好的性能

6、可以通过CMakeLists.txt修改boost库路径

7、服务通过systemctl监控启动

## 例子

参考src/main.cpp

## 编译

```
cmake --build /root/projects/HttpServer/cmake-build-debug --target HttpServer -- -j 4
```

## 压测

服务器4核CPU

```
$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://10.90.101.143:10873/IrcChatData/Test?aaaa=88"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     3.02ms    1.32ms  69.84ms   97.15%
    Req/Sec     8.44k   600.55    11.33k    76.67%
  338442 requests in 10.10s, 46.48MB read
Requests/sec:  33511.10
Transfer/sec:      4.60MB

$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://10.90.101.143:10873/IrcChatData/Test?aaaa=88"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.91ms    1.11ms  63.89ms   97.33%
    Req/Sec     8.73k   452.23    10.95k    78.47%
  350872 requests in 10.10s, 48.18MB read
Requests/sec:  34741.34
Transfer/sec:      4.77MB

$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://10.90.101.143:10873/IrcChatData/Test?aaaa=88"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.94ms    1.24ms  66.96ms   97.58%
    Req/Sec     8.66k   470.30    11.42k    77.23%
  347962 requests in 10.10s, 47.79MB read
Requests/sec:  34452.59
Transfer/sec:      4.73MB
```
