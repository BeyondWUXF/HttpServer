#!/bin/sh

usage() {
    echo "Usage: ./httpserver.sh reload limit"
    echo "       ./httpserver.sh start debug|test|product"
    echo "       ./httpserver.sh stop debug|test|product"
    echo "       ./httpserver.sh restart debug|test|product"
}

if [ "$1" == "reload" ]; then
    if [ "$2" == "limit" ]; then
        echo "reload limit"
        sh reload_limit.sh HttpServer
        echo "reload limit over"
    else
        usage
    fi
elif [ "$1" == "restart" ]; then
    if [ "$2" == "debug" ]; then
        echo "restart debug"
        systemctl restart HttpServer.debug
    elif [ "$2" == "test" ]; then
        echo "restart test"
        systemctl restart HttpServer.test
    elif [ "$2" == "product" ]; then
        echo "restart product"
        systemctl restart HttpServer.product
     else
        usage
     fi
elif [ "$1" == "start" ]; then
    if [ "$2" == "debug" ]; then
        echo "start debug"
        systemctl start HttpServer.debug
    elif [ "$2" == "test" ]; then
        echo "start test"
        systemctl start HttpServer.test
    elif [ "$2" == "product" ]; then
        echo "start product"
        systemctl start HttpServer.product
     else
        usage
     fi
elif [ "$1" == "stop" ]; then
    if [ "$2" == "debug" ]; then
        echo "stop debug"
        systemctl stop HttpServer.debug
    elif [ "$2" == "test" ]; then
        echo "stop test"
        systemctl stop HttpServer.test
    elif [ "$2" == "product" ]; then
        echo "stop product"
        systemctl stop HttpServer.product
     else
        usage
     fi
else
    usage
fi