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
RapidJSON           | v1.1.0

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

