#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

#include "socket.h"
#include "io_wrapper.h"
#include "httplib.h"

class Request{
public:
    std::string host;
    int port;
    std::string batch_file;
    int id;

    bool is_server_connect;
    socketfd_t server_fd;
    bool is_batch_file_open;
    std::fstream batch_file_stream;

    Request(const std::string& host, int port, const std::string& batch_file, int id);
    Request(const Request& copy);
    void connect_server(bool is_nonblocking);
    void open_batch_file();
    bool send_batch_file_data_to_server_once();
    std::string read_server_response(int count, bool is_nonblocking);
};

namespace cgi{
    std::map<std::string, std::string> http_get_parameters();
}

void print_html_before_content(const std::vector<Request>& all_requests);
void print_html_content(int id, std::string msg);
void print_html_after_content();

int main(int argc, char *argv[]){
    /*
     * 1. parse env QUERY_STRING
     * 2. check connection_server
     * 3. connect to server, and print response
     *    3.1. connect to server (non-blocking)
     *    3.2. read data from batch file, and send data as input to server
     *    3.3. get response from server, and print as html_version to stdout
     */

    std::map<std::string, std::string> query_parameters;
    query_parameters = cgi::http_get_parameters();

    std::vector<Request> all_requests;
    for( int i = 1; i <= 5; i++ ){
        std::string host_name = "h" + std::to_string(i);
        std::string port_name = "p" + std::to_string(i);
        std::string batch_file_name = "f" + std::to_string(i);
        if( query_parameters.count(host_name) > 0 ){
            std::string host = query_parameters[host_name];
            int port = std::stoi(query_parameters[port_name]);
            std::string batch_file = query_parameters[batch_file_name];
            
            all_requests.emplace_back(host, port, batch_file, all_requests.size());
        }
    }

    std::fstream err_log;
    err_log.open("cgi_error.log", std::fstream::out);
    for( const auto& req: all_requests ){
        err_log << req.host << std::endl;   
        err_log << req.port << std::endl;   
        err_log << req.batch_file << std::endl;   
    }

    /* part3 */
    /* initialization of every Request 
     * 1. make an connection
     * 2. open batch_file
     */

#if 0
    // single connection
    Request req = all_requests[0];

    req.connect_server(false);
    req.open_batch_file();
    req.send_batch_file_data_to_server();

    print_html_before_content(all_requests);

    while( 1 ){
        // req.send_batch_file_data_to_server_once();
        std::string msg;
        msg = req.read_server_response(1024, false);
        if( msg.empty() ){
            close(req.server_fd);
            break;
        }
        nl2br(msg);
        print_html_content(req.id, msg);
    }

    print_html_after_content();
#endif
    
    int request_num = all_requests.size();

    for( auto& req: all_requests ){
        req.open_batch_file();
        req.connect_server(true);
    }

    // std::cerr << "send out\n";

    /* recieve msg from server and print out */
    fd_set read_fds, write_fds;
    int max_fd = -1;
    FD_ZERO(&read_fds);
    for( const auto& req: all_requests ){
        if( req.is_server_connect ){
            FD_SET(req.server_fd, &read_fds);
            if( max_fd < req.server_fd ){
                max_fd = req.server_fd;
            }
        }
    }
    max_fd += 1;

    err_log << "request_num start:" << request_num << std::endl;

    print_html_before_content(all_requests);

    while( request_num > 0 ){
        fd_set select_read_fds = read_fds;
        int s_ret = 0;
        s_ret = select(max_fd, &select_read_fds, NULL, NULL, NULL);

        err_log << "\nSELECT return: " << s_ret << std::endl;
        err_log << "request_num:" << request_num << std::endl;

        if( s_ret < 0 )
            perror_and_exit("select error");

        for( auto& req: all_requests ){
            if( FD_ISSET(req.server_fd, &select_read_fds) ){

                err_log << "fd " << req.server_fd << " can be read\n";

                std::string msg;
                msg = req.read_server_response(1024, true);

                err_log << "msg: " << msg << "\n";

                if( msg.empty() ){
                    /* this server has no response */
                    err_log << "FD_CLR: " << req.port << std::endl;
                    FD_CLR(req.server_fd, &read_fds);
                    err_log << "FD_CLR finish: " << req.port << std::endl;
                    close(req.server_fd);
                    req.is_server_connect = false;
                    request_num--;
                    err_log << "request_num after clear:" << request_num << std::endl;
                    continue;
                }
                nl2br(msg);
                print_html_content(req.id, msg);

                // write to socket.
                req.send_batch_file_data_to_server_once();
            }
        }
    }

    err_log << "end of requests\n";
    print_html_after_content();
    err_log << "print_html_after_content() finish\n";

    err_log.close();
    return 0;
}

/* class Request */
Request::Request(const std::string& host, int port, const std::string& batch_file, int id){
    this->host = host;
    this->port = port;
    this->batch_file = batch_file;
    this->id = id;
    is_server_connect = false;
    is_batch_file_open = false;
}

Request::Request(const Request& copy){
    host = copy.host;
    port = copy.port;
    batch_file = copy.batch_file;
    id = copy.id;
    is_server_connect = false;
    is_batch_file_open = false;
}

void Request::connect_server(bool is_nonblocking){
    /* connect server */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( server_fd < 0 )
        perror_and_exit("create socket error");
    if( socket_connect(server_fd, host.c_str(), port) < 0 )
        perror_and_exit(("connect to " + host + ":" + std::to_string(port) + " error").c_str());
    is_server_connect = true;

    if( is_nonblocking ){
        int flags = fcntl(server_fd, F_GETFL, 0);
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    }
}

void Request::open_batch_file(){
    /* open batch_file */
    batch_file_stream.open(batch_file, std::fstream::in);
    if( !batch_file_stream ){
        perror_and_exit(("open batch_file " + batch_file + " error").c_str());
    }

    is_batch_file_open = true;
}

bool Request::send_batch_file_data_to_server_once(){
    if( !is_batch_file_open )
        return false;
    if( !is_server_connect )
        return false;
    
    std::string command;
    if( std::getline(batch_file_stream, command) ){
        command += "\n";
        int w_size = write_all(server_fd, command.c_str(), command.length());   
        if( w_size < 0 ){
            perror_and_exit("write error");
        }
        nl2br(command);
        print_html_content(id, "<b>" + command + "</b>");
        return true;
    }
    batch_file_stream.close();
    is_batch_file_open = false;
    return false;
}

std::string Request::read_server_response(int count, bool is_nonblocking){
    if( !is_server_connect )
        return "";

    std::string msg = str::read(server_fd, count, is_nonblocking);
    return msg;
}

namespace cgi{
    /* cgi-related api */
    std::map<std::string, std::string> http_get_parameters(){
        std::string query_string = getenv("QUERY_STRING");
        std::vector<std::string> parameter_vector;

        while( 1 ){
            /* bad smell string split, but it works */
            std::size_t spliter = query_string.find_first_of('&');
            if( spliter == std::string::npos ){
                parameter_vector.push_back(query_string);
                break;
            }

            std::string parameter = query_string.substr(0, spliter);
            query_string = query_string.substr(spliter+1, std::string::npos);
            parameter_vector.push_back(parameter);
        }

        std::map<std::string, std::string> query_parameters;
        for( const auto& parameter: parameter_vector ){
            std::size_t spliter = parameter.find_first_of('=');
            if( spliter == std::string::npos ){
                continue;
            }
            std::string key = parameter.substr(0, spliter);
            std::string value = parameter.substr(spliter+1, std::string::npos);
            if( value.empty() )
                continue;

            query_parameters[key] = value;
        }
        
        return query_parameters;
    }

}

void print_html_before_content(const std::vector<Request>& all_requests){
    int request_num = all_requests.size();

    std::string output;
    output += "<html>";
    output += "<head>";
    output += "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />";
    output += "    <title>Network Programming Homework 3</title>";
    output += "</head>";
    output += "<body bgcolor=#336699>";
    output += "    <font face=\"Courier New\" size=2 color=#FFFF99>";
    output += "        <table width=\"800\" border=\"1\">";
    output += "            <tr>";

    for( int i = 0; i < request_num; i++ ){
         output += "<td>" + all_requests[i].host + "</td>";
    }

    output += "            </tr>";
    output += "            <tr>";

    for( int i = 0; i < request_num; i++ ){
         output += "<td valign=\"top\" id=\"m" + std::to_string(i) + "\"></td>";
    }

    output += "            </tr>";
    output += "        </table>";
    output += "    </font>";

    std::cout << output;
}

void print_html_content(int id, std::string msg){
    std::cout << "<script>document.all['m" + std::to_string(id) + "'].innerHTML += '" + msg + "';</script>";
}

void print_html_after_content(){
    std::string output;
    output += "</body>"; 
    output += "</html>"; 
    std::cout << output;
}

