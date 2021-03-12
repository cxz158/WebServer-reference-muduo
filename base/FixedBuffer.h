#pragma once
#include <array> 
#include <string.h>
#include "noncopyable.h"

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer():cur_(data_) {}
    ~FixedBuffer() {}

    void append(const char* buf, size_t len)
    {
        if(avail() > static_cast<int>(len))
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }
    
    char* current() { return cur_; }
    int avail() const { return static_cast<int>(end() - cur_);}
    void add(size_t len) { cur_ += len; } 
    void init() { cur_ = data_;bzero(); }     //在muduo的源码这里命名为reset,
    void bzero() { memset(data_, 0, sizeof(data_)); }
    bool empty() { return cur_ == data_; }


private:
    const char* end() const { return data_ + sizeof(data_); }
    char data_[SIZE];
    char* cur_;
};

