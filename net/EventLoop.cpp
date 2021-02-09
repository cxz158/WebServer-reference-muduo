#include "EventLoop.h"
#include "../base/log.h"
#include <sys/epoll.h>
#include <assert.h>
#include <stdlib.h>

__thread EventLoop* t_loopInThisThread = nullptr;

EventLoop::EventLoop():threadId_(CurrentThread::tid()), looping_(false)
{
    if(t_loopInThisThread)
    {
        log_fatal("EventLoop created %ld in thread %d\n",this,threadId_);
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

    sleep(5);
    log("EventLoop %ld stop looping\n",this);
    looping_ = false;
}

void EventLoop::abortNotInLoopThread()
{
    if(!isInLoopThread())
        log_fatal("this loop 0x%x is not belong to thread %d\n",this,CurrentThread::tid());
}


