/** 
 * @file httplib.h
 * @brief HTTP protocol related library, like HTTP request, response.
 */

#ifndef __HTTPLIB_H__
#define __HTTPLIB_H__

#include <string>
#include <map>

const std::string HTTP_NEWLINE = "\r\n";

namespace http{
    enum HTTPRequestMethod{
        ERROR,
        GET,
        POST,
    };

    HTTPRequestMethod str_to_http_request_method(std::string http_method);
    std::string http_request_method_to_str(HTTPRequestMethod http_method);

    struct HTTPRequest{
        http::HTTPRequestMethod method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> header;
        std::string get_parameter_unparse;
        std::string post_parameter_unparse;

        HTTPRequest();
        void print();
        void print_header();
    };

    // int HTTPStatusCode;
    // 1xx Informational
    // 2xx Success
    //     200 OK
    // 3xx Redirection
    // 4xx Client Error
    //     403 Forbidden
    //     404 Not Found
    // 5xx Server Error
    //     500 Internal Server Error
    //     501 Not Implemented
    extern std::map<int, std::string> status_code_to_msg;

    struct HTTPResponse{
        std::string version;
        int status_code;
        std::map<std::string, std::string> header;

        HTTPResponse();
        HTTPResponse(std::string version, int status_code);
        std::string render_error_response_quick();
        std::string render_response_metadata(bool is_end_of_header);
        std::string render_response_header();
    };
}

void nl2br(std::string& str);

#endif
