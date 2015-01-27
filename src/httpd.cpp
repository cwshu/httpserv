#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "socket.h"
#include "io_wrapper.h"
#include "server_arch.h"
#include "string_more.h"
#include "httplib.h"

const char HTTPD_IP[] = "0.0.0.0";
const uint16_t HTTPD_DEFAULT_PORT = 80;

std::string DOCUMENT_ROOT;

void httpd_service(socketfd_t client_socket, SocketAddr& client_addr);
void read_and_parse_http_request(http::HTTPRequest& client_request, socketfd_t client_socket);
void static_content_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr);
void cgi_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr);

void initial_document_root(){
    /*
    char* home_dir = getenv("HOME");
    if(!home_dir)
        error_print_and_exit("Error: No HOME enviroment variable\n");

    snprintf(document_root, 1024, "%s/ras/", home_dir);*/
    int ret = chdir(DOCUMENT_ROOT.c_str());
    if(ret == -1)
        perror_and_exit("chdir error");
}

/*
const std::string CONFIG_FILE_NAME("httpd.conf");
std::string read_config_file_doc_root(){
    fstream config_file;
    config_file.open(CONFIG_FILE_NAME, std::ios::in);
    std::string config_record;
    std::getline(config_file, config_record);   
}
*/

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

    DOCUMENT_ROOT = getenv("HOME") + std::string("/workspace/NP_hw4_socks/test_build");
    initial_document_root();

    access_log.open("httpd-access.log", std::fstream::app);
    error_log.open("httpd-error.log", std::fstream::app);

    access_log << "access from: " << client_addr.to_str() << std::endl;

    // part 2
    http::HTTPRequest client_request;
    read_and_parse_http_request(client_request, client_socket);

    if( client_request.method != http::GET ){
         error_log << client_addr.to_str() << " => only support HTTP GET method now" << std::endl;
         return;
    }

    // part 3
    // check file exist
    if( access(client_request.path.c_str(), F_OK) == -1 ){
        error_log << client_addr.to_str() << " => " << client_request.path << " not found" << std::endl;
        std::string response = http::HTTPResponse(client_request.version, 404).render_response_metadata(true);
        write_all(client_socket, response.c_str(), response.length());
        return;
    }

    std::size_t found = client_request.path.find_last_of(".");
    std::string file_ext;
    if( found != std::string::npos ){
        file_ext = client_request.path.substr(found+1, std::string::npos);
    }
    std::string file_type = file_extension_to_type(file_ext);

    if( file_type == "cgi" ){
        cgi_handler(client_request, client_socket, client_addr);
    }
    else{
        static_content_handler(client_request, client_socket, client_addr);
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
    std::string path_get_frag = std::string(".") + strip(fetch_word(request_line, WHITESPACE));
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

void static_content_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr){
    access_log << client_addr.to_str() << " => static content handler " << std::endl;

    if( access(client_request.path.c_str(), R_OK) == -1 ){
        // can't read html file
        error_log << client_addr.to_str() << " => can't read " << client_request.path << std::endl;
        std::string response = http::HTTPResponse(client_request.version, 500).render_response_metadata(true);
        write_all(client_socket, response.c_str(), response.length());
        return;
    }

    // write response metadata
    std::string response = http::HTTPResponse(client_request.version, 200).render_response_metadata(true);
    write_all(client_socket, response.c_str(), response.length());
    // write data (html.file)
    int html_fd = open(client_request.path.c_str(), O_RDONLY);
    while( 1 ){
        std::string html_data = str::read(html_fd, 1024, false);
        if( html_data.empty() ){
            break;
        }
        write_all(client_socket, html_data.c_str(), html_data.length());
    }
}

void cgi_handler(http::HTTPRequest& client_request, socketfd_t client_socket, SocketAddr& client_addr){
    // 2.1. cgi handler: 9 env vars
    access_log << client_addr.to_str() << " => CGI handler " << std::endl;

    if( access(client_request.path.c_str(), X_OK) == -1 ){
        // can't read html file
        error_log << client_addr.to_str() << "=> can't execute " << client_request.path << std::endl;
        std::string response = http::HTTPResponse(client_request.version, 500).render_response_metadata(true);
        write_all(client_socket, response.c_str(), response.length());
        return;
    }

    // write response metadata
    http::HTTPResponse response = http::HTTPResponse(client_request.version, 200);
    std::string response_str = response.render_response_metadata(false);
    write_all(client_socket, response_str.c_str(), response_str.length());

    // run cgi and write response data
    int pid = fork();
    if( pid == 0 ){
        // redirect
        dup2(client_socket, 1);
        // env
        clearenv();
        setenv("QUERY_STRING", client_request.get_parameter_unparse.c_str(), 1);
        setenv("REQUEST_METHOD", http_request_method_to_str(client_request.method).c_str(), 1);
        setenv("SCRIPT_NAME", client_request.path.c_str(), 1);
        setenv("REMOTE_HOST", client_addr.ipv4_addr_str.c_str(), 1);
        setenv("REMOTE_ADDR", client_addr.ipv4_addr_str.c_str(), 1);
        // who cares
        setenv("CONTENT_LENGTH", "1024", 1);
        setenv("AUTH_TYPE", "1024", 1);
        setenv("REMOTE_USER", "1024", 1);
        setenv("REMOTE_IDENT", "1024", 1);
        // exec
        std::size_t found = client_request.path.find_last_of("/");
        std::string filename;
        if( found == std::string::npos ){
            filename = client_request.path;
        }
        else{
            filename = client_request.path.substr(found+1, std::string::npos);
        }
        execl(client_request.path.c_str(), filename.c_str(), NULL);
        perror("exec error");
        return;
    }
    else if( pid > 0 ){
        wait(NULL);
        return;
    }
    else{
        perror("fork error");
        return;
    }
}

std::string file_extension_to_type(std::string file_extension){
    /*
     * FileType: support now
     * 1. cgi
     * 2. default
     */
    if( file_extension == "cgi" ){
        return "cgi";
    }
    return "default";
}
