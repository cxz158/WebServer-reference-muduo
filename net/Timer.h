/******************************************************************
*File Name: Timer.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Thu 18 Feb 2021 09:58:46 PM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include "../base/log.h"
#include "../net/Channel.h"
#include <queue>
#include <functional>
#include <memory>
#include <unistd.h>
#include <sys/timerfd.h>


class EventLoop;
class TcpConnection;

using CallBack = std::function<void()>;

class Timer
{
public:
    Timer(CallBack callboack,int expiredTime, int interval); //定时单位:毫秒
    ~Timer() {}
    void run()const { timeoutCallBack_(); }
    
    long get_expiredTime() const { return expiredTime_; }
    int get_interval() const { return interval_; }
    void restart();

    void endisabled() { disabled_ = true; }
    bool disabled() { return disabled_; }

private:
    long expiredTime_;      //定时器过期时间,绝对时间
    const int interval_;        //循环定时间隔
    CallBack timeoutCallBack_;
    bool disabled_;
};

class Timerlarger
{
public:
    bool operator()(std::unique_ptr<Timer>& lhs, std::unique_ptr<Timer>& rhs) const 
    { 
        return lhs->get_expiredTime() > rhs->get_expiredTime();
    }
};

class TimerQueue : noncopyable
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue() { close(timefd_); };

    void addTimer(CallBack cb, int timeout, int interval);
    void removeTimer(Timer* timer);

private:
    using TimerPtr = std::unique_ptr<Timer>;
    void handleRead();
    void addTimerInLoop(Timer* timer);

    EventLoop* loop_;
    const int timefd_;
    Channel timerfdChannel_;
    std::priority_queue<TimerPtr, std::vector<TimerPtr>, Timerlarger> Timers_;  //优先级队列（最小堆）
};
