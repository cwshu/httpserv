/** 
 * @file utils.cpp
 * @brief helper functions and wrappers of standard library, POSIX api, and POSIX system call.
 *
 * Give some helper functions of low level api for easily usage, like POSIX system call write().
 */

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <string>

#include <errno.h>
#include <unistd.h>

#include "utils.h"

/* 
 * string helper functions 
 */

std::string fetch_word(std::string& parsed_str, const char* split_chars){
    /*
     * fetch the next "word" in parsed_str, and cut this word part in parsed_str.
     * the "word" means char sequence before split_chars;
     *
     * ex. 
     *
     * split_chars: rn;
     * parsed_str   => return_string, parsed_str_after
     * "rrrnn"      => ""           , ""
     * "rraarnbbbr" => "aa"         , "nbbbr"
     * "rraaabbbrn" => "aaabbb"     , ""
     */
    std::size_t start = parsed_str.find_first_not_of(split_chars);
    std::size_t end = parsed_str.find_first_of(split_chars, start);
    // std::size_t next_start = parsed_str.find_first_not_of(split_chars, end);
    if( start == std::string::npos ){
        /* no word, parsed_str only consist of split_chars  */
        parsed_str.clear();
        return std::string();
    }

    std::string word;
    if( end == std::string::npos ){
        word = parsed_str.substr(start, std::string::npos);
        parsed_str.clear();
    }
    else{
        word = parsed_str.substr(start, end - start);
        parsed_str.erase(0, end + 1);
    }

    return word;
}

std::string lstrip(const std::string& str){
    /* remove leading whitespaces of string */
    std::size_t found = str.find_first_not_of(WHITESPACE);
    if( found == std::string::npos ) 
        // nothing more than WHITESPACE
        return std::string();
    return str.substr(found, std::string::npos);
}

std::string rstrip(const std::string& str){
    /* remove trailing whitespaces of string */
    std::size_t found = str.find_last_not_of(WHITESPACE);
    if( found == std::string::npos ) 
        // nothing more than WHITESPACE
        return std::string();
    return str.substr(0, found+1);
}

std::string strip(const std::string& str){
    /* remove leading and trailing whitespaces of string */
    std::size_t start = str.find_first_not_of(WHITESPACE);
    std::size_t end = str.find_last_not_of(WHITESPACE);
    if( start == std::string::npos )
        // nothing more than WHITESPACE
        return std::string();
    return str.substr(start, end + 1 - start);
}

/* 
 * IO wrapper functions 
 */

void perror_and_exit(const char* str){
    perror(str);
    exit(EXIT_FAILURE);
}

/* print format string to stderr and exit */
void error_printf_and_exit(const char* format ... ){
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    /* exit */
    exit(EXIT_FAILURE);
}

/* write system call wrapper, it must write all data in buf to fd */
int write_all(int fd, const void* buf, size_t count){
    size_t writen_size = 0;
    const void* cur_buf = buf;
    while(writen_size < count){
        int size = write(fd, buf, count - writen_size);
        if(size < 0)
            return size;

        writen_size += size;
        cur_buf = (const char*)cur_buf + size;
    }
    return writen_size;
}

namespace str{

    /* wrapper of POSIX syscall read, return C++ std::string instead of memory buffer */
    std::string read(int fd, int count, bool is_nonblocking){

        char* buf = new char [count+1];
        int r_size;
        while( 1 ){
            r_size = ::read(fd, buf, count);
            if( r_size < 0 ){
                if( is_nonblocking )
                    if( errno == EAGAIN || errno == EWOULDBLOCK )
                        continue;

                if( errno == ECONNRESET ){
                    perror("read error");
                    return "";
                }

                perror_and_exit("read error");
            }
            break;
        }

        std::string read_str;
        if( r_size ){
            read_str = std::string(buf, r_size);
        }

        delete [] buf;
        return read_str;
    }
}
    
