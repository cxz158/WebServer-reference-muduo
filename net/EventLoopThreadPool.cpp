#include "EventLoopThreadPool.h"
#include <assert.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, int threadNums):
    baseloop_(baseloop),
    threadNums_(threadNums),
    next_(0),
    started_(false)
{
    if(threadNums <= 0)
        log_fatal("threadNums_ <= 0\n");
    start();
}

void EventLoopThreadPool::start()
{
    assert(!started_);
    baseloop_->assertInLoopThread();
    started_ = true;
    for(int i = 0; i < threadNums_; ++i)
    {
        std::string threadName = format_string("EventLoopThreadPool %d",i);        
        std::unique_ptr<EventLoopThread> t(new EventLoopThread(threadName));   
        threads_.push_back(std::move(t));
        loops_.push_back(threads_[i]->startLoop());
    }
}


