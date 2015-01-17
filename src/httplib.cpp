#include "httplib.h"

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
