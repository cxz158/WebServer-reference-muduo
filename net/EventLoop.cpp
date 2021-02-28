/******************************************************************
*File Name: Timer.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Thu 18 Feb 2021 09:58:46 PM PST
***************************************************************/
#include "EventLoop.h"
#include "../base/log.h"
#include "Epoll.h"
#include "Timer.h"
#include "Channel.h"
#include <assert.h>
#include <stdlib.h>
#include <sys/eventfd.h>

__thread EventLoop* t_loopInThisThread = nullptr;
const int POLLINTERVAL = 5000; //ms

int create_eventfd()
{
    int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(fd < 0)
        log_fatal("create eventfd failed\n");
    return fd;
}

EventLoop::EventLoop()
    : threadId_(CurrentThread::tid()),
      wakeupFd_(create_eventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      looping_(false),
      quit_(false),
      isdoPendingFunctors(false),
      epoller_(new Epoll(this)),
      timerQueue_(new TimerQueue(this)),
      mutex_()
{
    if(t_loopInThisThread)
    {
        log_fatal("EventLoop already created %ld in thread %d\n",this,threadId_);
    }
    else
    {
        t_loopInThisThread = this;
        wakeupChannel_->enableReading();
        epoller_->channel_add(*wakeupChannel_);
    }

}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = nullptr;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activeChannels_.clear();
        epoller_->poll(activeChannels_, POLLINTERVAL);
        for(auto it : activeChannels_)
        {
            it->handleEvent();
        }
        doPendingFunctors();     
    }
    
    log("EventLoop %ld stop looping\n",this);
    looping_ = false;
}

//操作channel
void EventLoop::addChannel(Channel& channel)
{
    epoller_->channel_add(channel);
}

void EventLoop::updateChannel(Channel& channel)
{
    epoller_->channel_mod(channel);
}

void EventLoop::removeChannel(Channel& channel)
{
    epoller_->channel_del(channel);
}


void EventLoop::runInLoop(const Functor& cb)
{
   if(isInLoopThread())
       cb();
   else
       queueInLoop(cb);
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb); 
    }
    if(!isInLoopThread() || isdoPendingFunctors)
        wakeup();
}

void EventLoop::addTimer(std::function<void()> cb, int timeout, int interval)
{ 
    timerQueue_->addTimer(cb, timeout, interval); 
}

void EventLoop::abortNotInLoopThread()
{
    if(!isInLoopThread())
        log_fatal("this loop 0x%x is not belong to thread %d\n",this,CurrentThread::tid());
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    size_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
        log_fatal("EventLoop::wakeup() failed! writes %d instead of 8 bytes\n",n);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    size_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
        log_fatal("EventLoop::handleRead() failed! reads %d instead of 8 bytes\n",n);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    isdoPendingFunctors = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(auto it : functors) it();
    
    isdoPendingFunctors = false;
}
