#!/bin/sh
#生成新日志文件

log_path=${1}
bin_name=${2}

cd ${log_path}
if [ -e main.log ]; then
    mv main.log main_`date "+%Y%m%d%H%M%S"`.log
fi
kill -12 `ps -ef|grep ${bin_name} | grep -v env|grep -v grep | awk '{print $2}'`