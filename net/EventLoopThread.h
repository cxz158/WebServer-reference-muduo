#pragma once
#include "EventLoop.h"
#include "../base/Thread.h"
#include "../base/noncopyable.h"

class EventLoopThread : noncopyable
{
public:
    EventLoopThread(std::string threadName);
    ~EventLoopThread();
    EventLoop* startLoop();
private:
    void threadfunc(); 

    Thread thread_;
    MutexLock mutex_;
    Condition cond_;

    EventLoop* loop_; //要在子线程中创建
    bool exiting_;
};

