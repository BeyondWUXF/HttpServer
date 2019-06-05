# HttpServer
C++实现基于boost库实现的http服务端

[boost](https://www.boost.org/users/history/version_1_69_0.html)

## 依赖

软件 |软件要求
------|--------
gcc版本:          	|   4.8.5及以上版本、glibc-devel
cmake版本：       	|   3.14.2及以上版本
c++版本：         |	c++11及以上版本
boost版本：         |  1.69.0
RapidJSON版本：           | v1.1.0
hiredis版本:      |v0.14.0及以上版本

## 说明

1、使用boost协程库实现并发，经过压力测试协程和异步的QPS没太大差别，选择用协程方式实现更易于代码编写

2、boost1.70.0官方上说有很大改动，理论上使用1.6*.0的版本都能兼容

3、使用boost日志库写日志方式采用异步方式，boost框架单独启一个线程进行写日志

4、使用静态链接方式链接boost库

5、由于网络IO，理论上将工程线程数设置为cpu*2能达到最好的性能

6、可以通过CMakeLists.txt修改boost库路径

7、服务通过systemctl监控启动

8、使用rapidjson库进行json的解析、封装

9、使用hiredis库实现redis客户端操作，使用阻塞方式，封装了线程安装的redis相关调用方法

## 例子

参考src/http_main.cpp

## 编译

```
cmake --build /root/projects/HttpServer/cmake-build-debug --target HttpServer -- -j 4
```

## 配置服务启动

```
// 创建项目文件夹
mkdir -p /data/htdocs/HttpServer/bin/       // 路径可自己选择
mkdir -p /data/htdocs/HttpServer/etc/

// 将可执行程序移到目录下
cp YOUR_DIR/cmake-build-debug/HttpServer /data/htdocs/HttpServer/bin/

// 将配置文件移到目录下
cp YOUR_DIR/etc/* /data/htdocs/HttpServer/etc/

// 创建systemctl配置文件
ln -s /data/htdocs/HttpServer/etc/HttpServer.debug.service /usr/lib/systemd/system/HttpServer.debug.service

// 配置文件重载
systemctl daemon-reload

// 启动服务
systemctl start HttpServer.debug

// 查看启动日志
journalctl -fu HttpServer.debug
```

## 压测（不使用JSON库）

虚拟机服务器4核CPU
4个线程处理http请求

```
压测用例：
curl http://1.1.1.1:123/IrcChatData/Test?aaaa=1 -H "appid: appid123"
响应：
{
    "uri": "/IrcChatData/Test",
    "param": "1",
    "appid": "appid123"
}
```

```
$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://1.1.1.1:123/IrcChatData/Test?aaaa=88"
Running 10s test @ http://1.1.1.1:123/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     3.02ms    1.32ms  69.84ms   97.15%
    Req/Sec     8.44k   600.55    11.33k    76.67%
  338442 requests in 10.10s, 46.48MB read
Requests/sec:  33511.10
Transfer/sec:      4.60MB

$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://1.1.1.1:123/IrcChatData/Test?aaaa=88"
Running 10s test @ http://1.1.1.1:123/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.91ms    1.11ms  63.89ms   97.33%
    Req/Sec     8.73k   452.23    10.95k    78.47%
  350872 requests in 10.10s, 48.18MB read
Requests/sec:  34741.34
Transfer/sec:      4.77MB

$ ./wrk -c 100 -d 10 -t 4 -H "appid: 123" "http://1.1.1.1:123/IrcChatData/Test?aaaa=88"
Running 10s test @ http://1.1.1.1:123/IrcChatData/Test?aaaa=88
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.94ms    1.24ms  66.96ms   97.58%
    Req/Sec     8.66k   470.30    11.42k    77.23%
  347962 requests in 10.10s, 47.79MB read
Requests/sec:  34452.59
Transfer/sec:      4.73MB
```

## 压测（使用rapidjson库）

虚拟机服务器4核CPU
4个线程处理http请求

```
压测用例：
curl -v -H "appid: appid123" -d '{"paString":"this is string", "paNumber":123, "paBool":true, "paNull":null, "paDouble":3.1415, "paArr":[1,2,3,4]}' "http://127.0.0.1:123/IrcChatData/rapidjson?aaaa=1"
响应：
< HTTP/1.1 200 OK
< Server: HttpServerV1.0
< Content-Type: application/json
< Content-Length: 169
< 
* Connection #0 to host 127.0.0.1 left intact
{"respon":"respon from server","param":"1","appid":"appid123","paString":"this is string","paNumber":123,"paBool":true,"paNull":null,"paDouble":3.1415,"paArr":[1,2,3,4]}
```

```
$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1"
Running 10s test @ http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.68ms   15.61ms 379.37ms   98.95%
    Req/Sec     3.41k   478.08     8.36k    97.25%
  270865 requests in 10.10s, 67.16MB read
Requests/sec:  26818.85
Transfer/sec:      6.65MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1"
Running 10s test @ http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.44ms   12.89ms 340.67ms   99.11%
    Req/Sec     3.36k   370.57     7.94k    96.65%
  269551 requests in 10.10s, 66.84MB read
Requests/sec:  26688.02
Transfer/sec:      6.62MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1"
Running 10s test @ http://1.1.1.1:123/IrcChatData/rapidjson?aaaa=1
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.66ms   13.66ms 355.83ms   99.08%
    Req/Sec     3.30k   365.51     7.25k    95.02%
  263780 requests in 10.10s, 65.41MB read
Requests/sec:  26116.92
Transfer/sec:      6.48MB
```

## 压测（使用hiredis，测试get）

虚拟机服务器4核CPU
4个线程处理http请求,每个线程单独的redis连接

```
压测用例：
curl -v -H "appid: appid123" -d '{"paString":"this is string", "paNumber":123, "paBool":true, "paNull":null, "paDouble":3.1415, "paArr":[1,2,3,4]}'  "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test123"
响应：
< HTTP/1.1 200 OK
< Server: HttpServerV1.0
< Content-Type: application/json
< Content-Length: 105
< 
* Connection #0 to host 10.90.101.143 left intact
{"uri":"/IrcChatData/redissafe", "param":"1", "appid":"appid123", "redis":"redis test key 123123 132321"}
```

```
$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test123"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test123
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    13.19ms   25.50ms 524.83ms   98.55%
    Req/Sec     2.36k   687.68    17.97k    95.47%
  186286 requests in 10.10s, 34.82MB read
Requests/sec:  18445.14
Transfer/sec:      3.45MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test123"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test123
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    12.93ms   25.66ms 533.21ms   98.54%
    Req/Sec     2.42k   493.28     8.77k    96.73%
  191743 requests in 10.10s, 35.84MB read
Requests/sec:  18984.99
Transfer/sec:      3.55MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test123"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test123
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    13.07ms   24.46ms 516.34ms   98.65%
    Req/Sec     2.34k   425.48     7.13k    96.36%
  185720 requests in 10.10s, 34.71MB read
Requests/sec:  18388.10
Transfer/sec:      3.44MB
```

压测redis value数据大小为10k

```
$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test124"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test124
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    18.89ms   31.38ms 609.96ms   98.39%
    Req/Sec     1.60k   402.11     7.16k    89.79%
  126766 requests in 10.00s, 1.20GB read
Requests/sec:  12670.37
Transfer/sec:    122.52MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test124"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test124
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    18.27ms   27.61ms 556.83ms   98.51%
    Req/Sec     1.61k   523.50     9.79k    90.30%
  127734 requests in 10.10s, 1.21GB read
Requests/sec:  12648.19
Transfer/sec:    122.30MB

$ /opt/tool/wrk/wrk -c 200 -d 10 -t 8 -s httpserver.lua "http://1.1.1.1:123/IrcChatData/redissafe?aaaa=1&key=test124"
Running 10s test @ http://10.90.101.143:10873/IrcChatData/redissafe?aaaa=1&key=test124
  8 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    18.20ms   27.26ms 556.10ms   98.57%
    Req/Sec     1.61k   488.57     9.75k    88.30%
  127723 requests in 10.10s, 1.21GB read
Requests/sec:  12647.38
Transfer/sec:    122.29MB
```
