#pragma once
#include "LoggingAsync.h"
#include <error.h>

namespace Log{

    extern pthread_once_t once_control_;
    extern std::unique_ptr<LoggingAsync> LoggingAsync_;
    extern std::string logFileName_;

    void once_init();

}

std::string format_string(const char* format, ...);

template<typename ...Args>
void log(const char* format,Args... args)
{
    pthread_once(&Log::once_control_, Log::once_init);
    
    std::string str = format_string(format, args...);
    Log::LoggingAsync_->append(str);
}

template<typename ...Args>
void log_syserr(const char* format,Args... args)
{
    log(format, args...);
    log("error = %s\n",strerror(errno));
}

//发生致命错误，将日志写入后，abort();
template<typename ...Args>
void log_fatal(const char* format,Args... args)
{
    log(format, args...);
    Log::LoggingAsync_->stop();
    abort();
}

template<typename ...Args>
void log_error_fatal(const char* format,Args... args)
{
    log_syserr(format, args...);
    Log::LoggingAsync_->stop();
    abort();
}

