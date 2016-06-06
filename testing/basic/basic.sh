#!/bin/sh

echo -e "Test HTTP GET: "
nc localhost 8100 < testing/basic/get.http_req
echo -e "\nTest HTTP GET 403: "
nc localhost 8100 < testing/basic/get403.http_req
echo -e "\nTest HTTP GET 404: "
nc localhost 8100 < testing/basic/get404.http_req
echo -e "\nTest HTTP GET 501: "
nc localhost 8100 < testing/basic/get501.http_req
