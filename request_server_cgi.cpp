#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket.h"
#include "io_wrapper.h"
#include "httplib.h"

class request{
public:
    std::string host;
    int port;
    std::string batch_file;

    request(const std::string& host, int port, const std::string& batch_file){
        this->host = host;
        this->port = port;
        this->batch_file = batch_file;
    }
};

namespace cgi{
    std::map<std::string, std::string> http_get_parameters();
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
            
            all_requests.emplace_back(host, port, batch_file);
        }
    }

    /*
    for( const auto& req: all_requests ){
        std::cout << req.host << std::endl;   
        std::cout << req.port << std::endl;   
        std::cout << req.batch_file << std::endl;   
    }*/
    
    /* part3 */
    /* connect server */
    auto req = all_requests[0];
    socketfd_t request_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( request_socket < 0 )
        perror_and_exit("create socket error");
    if( socket_connect(request_socket, req.host.c_str(), req.port) < 0 )
        perror_and_exit("connect error");

    /* open batch_file */
    std::fstream batch_file;
    batch_file.open(req.batch_file, std::fstream::in | std::fstream::binary);
    if( !batch_file ){
        perror_and_exit("open batch_file error");
    }

    /* read from batch file and write to request server */
    batch_file.seekg(0, batch_file.end);
    int file_size = batch_file.tellg();
    batch_file.seekg(0, batch_file.beg);

    while( file_size > 0 ){
        char buf[1024];
        int r_size;
        r_size = (file_size > 1024) ? 1024 : file_size;
        file_size -= r_size;
        // std::cout << "r_size: " << r_size << std::endl;

        batch_file.read(buf, r_size);
        int w_size = write_all(request_socket, buf, r_size);   
        if( w_size < 0 ){
            perror_and_exit("write error");
        }
    }

    /* recieve msg from server and print out */
    print_html_before_content(all_requests);
    std::string msg;
    while( 1 ){
        msg = str::read(request_socket, 1024);
        if( msg.empty() ) break;
        nl2br(msg);
        print_html_content(0, msg);
    }
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
