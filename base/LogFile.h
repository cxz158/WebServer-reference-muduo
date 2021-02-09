#pragma once
#include <memory>
#include "Mutex.h"
#include "noncopyable.h"
#include "FileUtil.h"

//日志文件类，提供自动归档功能，
//每被append flushEveryN 次就会自动flush
class LogFile :noncopyable
{
public:
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile(){}; //* 可能需要放到.cpp 中定义

    void append(const char* logline, int len);
    void flush();
private:
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<FileUtil> file_;
};

