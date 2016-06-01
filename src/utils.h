#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

/* string helper functions */

std::string fetch_word(std::string& parsed_str, const char* split_chars);

const char WHITESPACE[] = " \t\r\n\v\f";

/** \brief remove leading whitespaces of string */
std::string lstrip(const std::string& str);
/** \brief remove trailing whitespaces of string */
std::string rstrip(const std::string& str);
/** \brief remove leading and trailing whitespaces of string */
std::string strip(const std::string& str);

/* IO wrapper functions */

void perror_and_exit(const char* str);
/** \brief print format string to stderr and exit */
void error_printf_and_exit(const char* format ... );

/** \brief write system call wrapper, it must write all data in buf to fd */
int write_all(int fd, const void* buf, size_t count);

namespace str{
    /** \brief wrapper of POSIX syscall read, return C++ std::string instead of memory buffer */
    std::string read(int fd, int count, bool is_nonblocking);
}

#endif /* end of include guard: __UTILS_H__ */
