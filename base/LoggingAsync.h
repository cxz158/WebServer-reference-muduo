#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "CountDownLatch.h"
#include "FixedBuffer.h"
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"

class LoggingAsync
{
public:
    LoggingAsync(const std::string basename, int flushInterval = 2);
    ~LoggingAsync(){ if(running_) stop(); };

    void start(){
        running_ = true;
        thread_.start();
        latch_.wait();
        std::cout<<"LogginAsync started"<<std::endl;
    }

    void stop(){
        running_ = false;
        cond_.notify_one();     //提醒后台进程，即将关闭日志，将buffer中的内容赶紧写入日志文件
        thread_.join();
        std::cout<<"LogggingAsysnc stoped"<<std::endl;
    }

    void append(const char* logline, int len); //前端使用append 写日志进buffer
private:
    void threadFunc();                        //后端 另起线程 将buffer中的内容写入日志文件

    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;
    
    const int flushInterval_;
    bool running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    CountDownLatch latch_;

    BufferPtr currentBuffer_;   //当前缓冲
    BufferPtr nextBuffer_;      //预备缓冲
    BufferVector buffers_;      //待写入文件的已填满的缓冲

};

