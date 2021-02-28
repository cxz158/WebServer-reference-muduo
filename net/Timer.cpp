/******************************************************************
*File Name: /home/cxz/code/mywork/net/Timer.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Fri 19 Feb 2021 06:22:11 AM PST
***************************************************************/
#include "Timer.h"
#include "EventLoop.h"
#include <cstdio>

const int TIMERINTERVAL = 3;  //s timerfd 被唤醒的时间间隔

inline
long delay_expiredTime(int delay)
{
    struct timeval now;
    gettimeofday(&now, nullptr);
    long ret =  now.tv_sec * 1000 + now.tv_usec / 1000 + delay;
    return ret;
}

Timer::Timer(CallBack callback,int expiredTime, int interval)
    : expiredTime_(delay_expiredTime(expiredTime)),
      interval_(interval),
      timeoutCallBack_(callback),
      disabled_(false)
{  }


inline
void Timer::restart()
{
    expiredTime_ = delay_expiredTime(interval_);
}


int create_timerfd()
{
    int timefd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if(timefd < 0)
        log_fatal("timefd created failed\n");
    return timefd;
}


void start_timerfd(const int& timerfd)
{
    struct itimerspec new_value;
    bzero(&new_value, sizeof new_value);
    new_value.it_value.tv_sec = TIMERINTERVAL;
    new_value.it_interval.tv_sec = TIMERINTERVAL;
    int ret = timerfd_settime(timerfd, 0, &new_value, nullptr);
    if(ret < 0)
        log_fatal("timerfd_settime failed\n");
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timefd_(create_timerfd()),
      timerfdChannel_(loop, timefd_)
{
    timerfdChannel_.enableReading();
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    loop_->addChannel(timerfdChannel_);
    start_timerfd(timefd_);
}

void TimerQueue::addTimer(CallBack cb, int timeout, int interval)
{
   Timer* timer(new Timer(cb, timeout, interval)); 
   loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

void TimerQueue::handleRead()
{
    uint64_t one = 1;
    size_t n = read(timefd_, &one, sizeof one);
    if(n != sizeof one)
        log_fatal("read from timefd_ %d bytes instead of 8 bytes\n", n);

    struct timeval now;
    gettimeofday(&now, nullptr);
    long long curTime = now.tv_sec * 1000 + now.tv_usec / 1000;
    while(!Timers_.empty() && Timers_.top()->get_expiredTime() < curTime)
    {
        if(!Timers_.top()->disabled())
        {
            Timers_.top()->run();
        }

        if(Timers_.top()->get_interval())
        {
            Timer* tmp = new Timer(*Timers_.top());
            tmp->restart();
            addTimerInLoop(tmp);
        }
        Timers_.pop();
    }
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();
    Timers_.push(TimerPtr(timer));
}
