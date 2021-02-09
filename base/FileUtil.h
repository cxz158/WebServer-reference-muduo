#pragma once
#include "noncopyable.h"
#include <string>
#include <functional>

class FileUtil
{
public:
    explicit FileUtil(std::string filename);
    ~FileUtil();

    void append(const char* logline, const size_t len);
    void flush();
    const FILE* get(){return fp_;}
private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64 * 1024];
};

