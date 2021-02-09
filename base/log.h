#pragma once
#include "LoggingAsync.h"
#define BUFSIZE 1024

namespace Log{

    extern pthread_once_t once_control_;
    extern std::unique_ptr<LoggingAsync> LoggingAsync_;
    extern std::string logFileName_;

    void once_init();

}

//使用变参模板做成printf(fmt,...)风格的日志
template<typename ...Args>
void log(const char* format,Args... args)
{
    pthread_once(&Log::once_control_, Log::once_init);
    constexpr size_t oldlen = BUFSIZE;
    char buffer[oldlen];
    size_t newlen = snprintf(buffer, oldlen, format, args...);
    newlen++;    //加上末尾'\0'的长度

    if(newlen > oldlen)
    {
        std::vector<char> newbuffer(newlen);
        snprintf(newbuffer.data(), newlen, format, args...);
        Log::LoggingAsync_->append(newbuffer.data(), newlen);
    }
    else
    {
        Log::LoggingAsync_->append(buffer, newlen-1);
    }
}

