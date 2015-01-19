#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <unistd.h>

#include "socket.h"
#include "io_wrapper.h"
#include "server_arch.h"
#include "string_more.h"
#include "httplib.h"

const char HTTPD_IP[] = "0.0.0.0";
const uint16_t HTTPD_DEFAULT_PORT = 80;

void httpd_service(socketfd_t client_socket, SocketAddr& client_addr);
void read_and_parse_http_request(http::HTTPRequest& client_request, socketfd_t client_socket);
void http_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr);
void cgi_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr);

std::string file_extension_to_type(std::string file_extension);


int main(int argc, char *argv[]){
    SocketAddr httpd_addr(HTTPD_IP, HTTPD_DEFAULT_PORT);
    if(argc == 2){
        httpd_addr.port_hbytes = strtol(argv[1], NULL, 0);
    }

    socketfd_t httpd_listen_socket = bind_and_listen_tcp_socket(httpd_addr);
    start_multiprocess_server(httpd_listen_socket, httpd_service);

    return 0;
}

std::fstream access_log, error_log;
void httpd_service(socketfd_t client_socket, SocketAddr& client_addr){
    /* 
     * 1. start fork-based http server.
     * 2. recieve and parsing http request. 
     *    2.1. http method, path, version.
     *    2.2. header into map.
     *    2.3. content will be processed by file handler. XXX: not implement now.
     * 3. process path, find a file existence and choose file handler.
     * 4. cgi and http file handler.
     * 5. http response (status code) ... etc.
     */

    access_log.open("httpd-access.log", std::fstream::app);
    error_log.open("httpd-error.log", std::fstream::app);

    access_log << "access from: " << client_addr.to_str() << std::endl;

    // part 2
    http::HTTPRequest client_request;
    read_and_parse_http_request(client_request, client_socket);

    if( client_request.method != http::GET ){
         error_log << client_addr.to_str() << "=> only support HTTP GET method now" << std::endl;
         return;
    }

    // part 3
    // check file exist
    if( access(client_request.path.c_str(), F_OK) == -1 ){
        error_log << client_addr.to_str() << "=> " << client_request.path << " not found" << std::endl;
        std::string response = http::HTTPResponse(client_request.version, 404).render_response_metadata();
        write_all(client_socket, response.c_str(), response.length());
        return;
    }

    std::size_t found = client_request.path.find_last_of(".");
    std::string file_ext;
    if( found != std::string::npos ){
        file_ext = client_request.path.substr(found+1, std::string::npos);
    }
    std::string file_type = file_extension_to_type(file_ext);

    if( file_type == "http" ){
        http_handler(client_request, client_socket, client_addr);
    }
    else if( file_type == "cgi" ){
        cgi_handler(client_request, client_socket, client_addr);
    }
}

std::string NEWLINE = "\r\n";
std::string DOUBLE_NEWLINE = NEWLINE + NEWLINE;
const int MAX_BYTE_PER_LINE = 65536;

void read_and_parse_http_request(http::HTTPRequest& client_request, socketfd_t client_socket){
    /*
     * XXX: not parsing http request data section.
     *
     * HTTP request
     * GET dir/subdir/test.cgi?k1=v1&k2=v2&k3=v3#fragment HTTP/1.1
     * Host: localhost
     * \r\n
     */
    std::string request_msg;
    int msg_size = 0;
    while( 1 ){
        std::string request_msg_tmp = str::read(client_socket, MAX_BYTE_PER_LINE - msg_size, false);
        request_msg += request_msg_tmp;
        msg_size += request_msg_tmp.length();

        std::size_t found = request_msg.find(DOUBLE_NEWLINE);
        if( found != std::string::npos ){
            break;
        }
        if( msg_size >= MAX_BYTE_PER_LINE ){
            break;
        }
    }
    
    std::vector<std::string> request_msg_per_line;
    while( 1 ){
        /* transform http_request msg to vector of msg per line 
         * request_msg => request_msg_per_line.
         */
        /* bad smell string split, but it works */
        std::size_t spliter = request_msg.find(NEWLINE);
        if( spliter == std::string::npos ){
            request_msg_per_line.push_back(request_msg);
            break;
        }

        std::string one_line_msg = request_msg.substr(0, spliter);
        request_msg = request_msg.substr(spliter+NEWLINE.length(), std::string::npos);
        request_msg_per_line.push_back(one_line_msg);
    }

    std::string request_line = request_msg_per_line[0];
    access_log << "request_line: " << request_line << std::endl;
    request_msg_per_line.erase(request_msg_per_line.begin());
    // parse request_line
    std::string http_method = strip(fetch_word(request_line, WHITESPACE)) ;
    client_request.method = http::str_to_http_request_method(http_method);
    std::string path_get_frag = strip(fetch_word(request_line, WHITESPACE));
    client_request.path = strip(fetch_word(path_get_frag, "?"));
    client_request.get_parameter_unparse = strip(fetch_word(path_get_frag, "#"));
    client_request.version = strip(request_line);

    for (auto& header_one_line : request_msg_per_line) {
        std::string key = strip(fetch_word(header_one_line, ":"));
        std::string value = strip(header_one_line);
        if( key.empty() ){
            break;
        }
        client_request.header[key] = value;
    }

    client_request.print();
}

void http_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr){

}

void cgi_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr){
    // 2.1. cgi handler: 9 env vars
}

std::string file_extension_to_type(std::string file_extension){
    /*
     * FileType: support now
     * 1. html (default)
     * 2. cgi
     */
    if( file_extension == "cgi" ){
        return "cgi";
    }
    return "html";
}
