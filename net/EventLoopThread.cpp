#include "EventLoopThread.h"
#include <functional>
#include <assert.h>

EventLoopThread::EventLoopThread(std::string threadName)
    :thread_(std::bind(&EventLoopThread::threadfunc,this), threadName),
     mutex_(),
     cond_(mutex_),
     loop_(nullptr),
     exiting_(false)
{   }


EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start(); 
    {
        MutexLockGuard lock(mutex_);
        while(loop_ == nullptr) // 等待线程初始话完成
            cond_.wait();
    }
    return loop_;
}

void EventLoopThread::threadfunc()
{
   EventLoop loop;
   {
       MutexLockGuard lock(mutex_);
       loop_ = &loop; 
       cond_.notify_all();
   }
   loop.loop();
   loop_ = nullptr;
}

