#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#include "socket.h"
#include "io_wrapper.h"
#include "httplib.h"

class request{
public:
    std::string host;
    int port;
    std::string batch_file;
    int id;

    bool is_server_connect;
    socketfd_t server_fd;
    bool is_batch_file_open;
    std::fstream batch_file_stream;
    int rest_file_size;

    request(const std::string& host, int port, const std::string& batch_file, int id){
        this->host = host;
        this->port = port;
        this->batch_file = batch_file;
        this->id = id;
        is_server_connect = false;
        is_batch_file_open = false;
        rest_file_size = -1;
    }

    request(const request& copy){
        host = copy.host;
        port = copy.port;
        batch_file = copy.batch_file;
        id = copy.id;
        is_server_connect = false;
        is_batch_file_open = false;
        rest_file_size = -1;
    }

    void connect_server(){
        /* connect server */
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if( server_fd < 0 )
            perror_and_exit("create socket error");
        if( socket_connect(server_fd, host.c_str(), port) < 0 )
            perror_and_exit(("connect to " + host + ":" + std::to_string(port) + " error").c_str());
        is_server_connect = true;
    }

    void open_batch_file(){
        /* open batch_file */
        batch_file_stream.open(batch_file, std::fstream::in | std::fstream::binary);
        if( !batch_file_stream ){
            perror_and_exit(("open batch_file " + batch_file + " error").c_str());
        }

        is_batch_file_open = true;
        batch_file_stream.seekg(0, batch_file_stream.end);
        rest_file_size = batch_file_stream.tellg();
        batch_file_stream.seekg(0, batch_file_stream.beg);
    }

    void send_batch_file_data_to_server(){
        if( !is_batch_file_open )
            return;
        if( !is_server_connect )
            return;

        /* read from batch file and write to request server */
        while( rest_file_size > 0 ){
            send_batch_file_data_to_server_once();
        }

        shutdown(server_fd, 1); // closing to write;
        // close(server_fd);
    }

    void send_batch_file_data_to_server_once(){
        if( !is_batch_file_open )
            return;
        if( !is_server_connect )
            return;
        if( rest_file_size <= 0 )
            return;

        char buf[1024];
        int r_size;
        r_size = (rest_file_size > 1024) ? 1024 : rest_file_size;
        rest_file_size -= r_size;
        // std::cout << "r_size: " << r_size << std::endl;

        batch_file_stream.read(buf, r_size);
        int w_size = write_all(server_fd, buf, r_size);   
        if( w_size < 0 ){
            perror_and_exit("write error");
        }
    }

    std::string read_server_response(int count){
        if( !is_server_connect )
            return "";

        std::string msg = str::read(server_fd, count);
        return msg;
    }
};

namespace cgi{
    std::map<std::string, std::string> http_get_parameters();
}

void print_html_before_content(const std::vector<request>& all_requests);
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

    std::vector<request> all_requests;
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

    /*
    for( const auto& req: all_requests ){
        std::cout << req.host << std::endl;   
        std::cout << req.port << std::endl;   
        std::cout << req.batch_file << std::endl;   
    }
    */

    /* part3 */
    /* initialization of every request 
     * 1. make an connection
     * 2. open batch_file
     */
    for( auto& req: all_requests ){
        req.connect_server();
        req.open_batch_file();
    }
    for( auto& req: all_requests ){
        req.send_batch_file_data_to_server();
    }

    // std::cout << "send out\n";

    /* recieve msg from server and print out */
    fd_set read_fds;
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

    print_html_before_content(all_requests);

    int s_ret = 0;
    while( (s_ret = select(max_fd, &read_fds, NULL, NULL, NULL)) > 0 ){
        // std::cout << "\nSELECT return: " << s_ret << std::endl;

        for( auto& req: all_requests ){
            if( FD_ISSET(req.server_fd, &read_fds) ){
                std::string msg;
                msg = req.read_server_response(1024);
                if( msg.empty() ){
                    /* this server has no response */
                    // std::cout << "\nFD_CLR: " << req.port << std::endl;
                    FD_CLR(req.server_fd, &read_fds);
                    close(req.server_fd);
                    req.is_server_connect = false;
                    continue;
                }
                // std::cout << "msg: " << msg << std::endl;
                nl2br(msg);
                print_html_content(req.id, msg);
            }
        }
    }
    // std::cout << "\nSELECT return: " << s_ret << std::endl;

    print_html_after_content();
    
    return 0;
}

namespace cgi{
    /* cgi */
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
            query_parameters[key] = value;
        }
        
        return query_parameters;
    }

}

void print_html_before_content(const std::vector<request>& all_requests){
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

