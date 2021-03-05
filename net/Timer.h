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
    Timer(CallBack callboack, int interval); //定时单位:毫秒
    ~Timer() {}
    void run()const { timeoutCallBack_(); }
    
    long get_expiredTime() const { return expiredTime_; }

    void disabled() { abled_ = false; }
    bool abled() { return abled_; }
    
private:
    long expiredTime_;      //定时器过期时间,绝对时间
    CallBack timeoutCallBack_;
    bool abled_;
};

class Timerlarger
{
public:
    bool operator()(std::shared_ptr<Timer>& lhs, std::shared_ptr<Timer>& rhs) const 
    { 
        return lhs->get_expiredTime() > rhs->get_expiredTime();
    }
};

class TimerQueue : noncopyable
{
public:
    using TimerSptr = std::shared_ptr<Timer>;
    
    TimerQueue(EventLoop* loop);
    ~TimerQueue() { close(timefd_); };

    TimerSptr addTimer(CallBack cb, int interval);
private:
    void handleRead();
    void addTimerInLoop(TimerSptr timer);

    EventLoop* loop_;
    const int timefd_;
    Channel timerfdChannel_;
    std::priority_queue<TimerSptr, std::vector<TimerSptr>, Timerlarger> Timers_;  //优先级队列（最小堆）
};
