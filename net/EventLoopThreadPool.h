#pragma once
#include "EventLoopThread.h"
#include "../base/log.h"
#include <vector>


class EventLoopThreadPool
{
public:
    EventLoopThreadPool(EventLoop* baseloop, int threadNums);
    ~EventLoopThreadPool() { log("~EventLoopThreadPool()"); }
    
    void start();

    EventLoop* getNextLoop() {
        next_ = (next_+1)%threadNums_;
        return loops_[next_];
    }
private:
    EventLoop* baseloop_; //主事件循环 
    int threadNums_;
    int next_; //always in loop thread;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    bool started_;
};

