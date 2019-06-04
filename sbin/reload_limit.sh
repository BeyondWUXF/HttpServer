#!/bin/sh
#重新加载限流配置文件

kill -10 `ps -ef|grep ${1} | grep -v env|grep -v grep | awk '{print $2}'`