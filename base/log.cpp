#include "log.h"
#include <cstdarg>
#include <stdio.h>
#include <iostream>

namespace Log{

    pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
    std::unique_ptr<LoggingAsync> LoggingAsync_;
    std::string logFileName_ = "./httpSever.log";

    void once_init()
    {
        LoggingAsync_.reset(new LoggingAsync(logFileName_));
        LoggingAsync_->start();
    }

}

const int BUFSIZE = 1024;
__thread char buffer[BUFSIZE];

std::string format_string(const char* format, ...)
{
    va_list ap;
    va_start(ap,format);
    size_t newlen = vsnprintf(buffer, BUFSIZE, format, ap);
    va_end(ap);

    if(newlen >= BUFSIZE)
    {
        std::vector<char> newbuffer(newlen-1);
        va_list ap2;
        va_start(ap2, format);
        vsprintf(newbuffer.data(), format, ap2);
        va_end(ap2);
        return newbuffer.data();
    }
    return buffer;
}

/* //相比上一个format_string 利用RVO减少了一次拷贝， 但实际上提升的速度好像并不明显 */
/* std::string format_string(const char* format, ...) */
/* { */
/*     std::string ret; */
/*     va_list ap; */
/*     va_start(ap,format); */
/*     size_t newlen = vsnprintf(buffer, BUFSIZE, format, ap); */
/*     va_end(ap); */
/*     ret.resize(newlen); */

/*     if(newlen+1 > BUFSIZE) //加上'/0' */
/*     { */
/*         va_list ap2; */
/*         va_start(ap2, format); */
/*         vsprintf(const_cast<char*>(ret.data()), format, ap2); */
/*         va_end(ap2); */
/*     } */
/*     else */
/*     { */
/*         strcpy(const_cast<char*>(ret.data()), buffer); */
/*     } */
/*     return ret; */
/* } */
