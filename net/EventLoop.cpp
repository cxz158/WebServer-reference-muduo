#include "EventLoop.h"
#include <sys/epoll.h>
#include <assert.h>
#include <stdlib.h>

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop():looping_(false), threadId_(CurrentThread::tid())
{
    if(t_loopInThisThread)
    {
        //日志输出        
    }
    else
    {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopofCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    //
    looping_ = false;
}

void EventLoop::abortNotInLoopThread()
{
    if(!isInLoopThread())
        abort();
}
