/** 
 * @file httplib.cpp
 * @brief HTTP protocol related library, like HTTP request, response.
 */

#include <iostream>

#include "httplib.h"

namespace http{
    std::map<int, std::string> status_code_to_msg = {
        {200, "OK"},
        {403, "FORBIDDEN"},
        {404, "NOT FOUND"},
        {500, "INTERNAL SERVER ERROR"},
        {501, "NOT INPLEMENTED"},
    };

    HTTPRequestMethod str_to_http_request_method(std::string http_method){
        if( http_method == "GET" ) return http::GET;
        if( http_method == "POST" ) return http::POST;
        return http::ERROR;
    }

    std::string http_request_method_to_str(HTTPRequestMethod http_method){
        if( http_method == http::GET ) return "GET";
        if( http_method == http::POST ) return "POST";
        if( http_method == http::ERROR ) return "ERROR";
        return "ERROR";
    }

    // HTTPRequest
    HTTPRequest::HTTPRequest(){
        method = http::ERROR;
    }

    void HTTPRequest::print(){
        std::cout << "HTTP method: " << http_request_method_to_str(method) << std::endl;
        std::cout << "path = " << path << std::endl;
        std::cout << "version = " << version << std::endl;
        std::cout << "get_parameters: " << get_parameter_unparse << std::endl;
        print_header();
        std::cout << std::endl;
    }

    void HTTPRequest::print_header(){
        for (const auto& kv_pair : header) {
            std::cout << kv_pair.first << " => " << kv_pair.second << std::endl;
        }
    }

    // HTTPResponse
    HTTPResponse::HTTPResponse(){
        status_code = 500;
    }

    std::string HTTPResponse::render_error_response_quick(){
        /*
        "HTTP/1.1 404 Not Found\n"
        "Content-Type: text/plain;\n"
        "Content-Length: 13\n"
        "\n"
        "404 Not Found"
        */

        std::string status_line = version + std::string(" ") + std::to_string(status_code) + std::string(" ") + status_code_to_msg[status_code] + HTTP_NEWLINE;
        std::string content = std::to_string(status_code) + std::string(" ") + status_code_to_msg[status_code];

        std::string header = "Content-Type: text/plain" + HTTP_NEWLINE;
        header += "Content-Length: " + std::to_string(content.length()) + HTTP_NEWLINE;
        header += HTTP_NEWLINE;

        std::string response = status_line + header + content;
        return response;
    }

    HTTPResponse::HTTPResponse(std::string version, int status_code){
        this->version = version;
        this->status_code = status_code;
    }

    std::string HTTPResponse::render_response_metadata(bool is_end_of_header){
        std::string response;

        response = version + std::string(" ");
        response += std::to_string(status_code) + std::string(" ") + status_code_to_msg[status_code];
        response += HTTP_NEWLINE;

        response += render_response_header();
        if( is_end_of_header ){
            response += HTTP_NEWLINE;
        }
        return response;
    }

    std::string HTTPResponse::render_response_header(){
        std::string response_header;
        for (const auto& kv_pair : header) {
            response_header += kv_pair.first + std::string(": ") + kv_pair.second + HTTP_NEWLINE;
        }
        return response_header;
    }
}

void nl2br(std::string& str){
    std::string::size_type pos = 0;
    while ((pos = str.find("\r\n", pos)) != std::string::npos){
        str.replace(pos, 2, "<br />");
    }

    pos = 0;
    while ((pos = str.find("\r", pos)) != std::string::npos){
        str.replace(pos, 1, "<br />");
    }
    
    pos = 0;
    while ((pos = str.find("\n", pos)) != std::string::npos){
        str.replace(pos, 1, "<br />");
    }
}
