#include "httplib.h"

void nl2br(std::string& str){
    std::string::size_type pos = 0;
    while ((pos = str.find("\n", pos)) != std::string::npos){
        str.replace(pos, 1, "<br>");
    }
}
