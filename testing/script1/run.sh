#!/bin/sh

echo -e "Test HTTP GET: "
nc localhost $1 < testing/script1/get.http
echo -e "\nTest HTTP GET 403: "
nc localhost $1 < testing/script1/get403.http
echo -e "\nTest HTTP GET 404: "
nc localhost $1 < testing/script1/get404.http
echo -e "\nTest HTTP GET 501: "
nc localhost $1 < testing/script1/get501.http
echo -e "\nTest Query String, ignore it, just same as Test HTTP GET: "
nc localhost $1 < testing/script1/querystring.http
