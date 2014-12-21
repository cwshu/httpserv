#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>

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

    for( const auto& req: all_requests ){
        std::cout << req.host << std::endl;   
        std::cout << req.port << std::endl;   
        std::cout << req.batch_file << std::endl;   
    }
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
