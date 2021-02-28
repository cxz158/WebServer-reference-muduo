/******************************************************************
*File Name: /home/cxz/code/mywork/net/Timer.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Fri 19 Feb 2021 06:22:11 AM PST
***************************************************************/
#pragma once
#include <sys/types.h>
#include <vector>
#include <memory>
#include <functional>
#include <queue>
#include "../base/CurrentThread.h"
#include "../base/noncopyable.h"
#include "../base/Mutex.h"

class Channel;
class Epoll;
class TimerQueue;
//一个线程只允许有一个EventLoop
class EventLoop: noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    
    void loop();
    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    void quit()
    { 
        quit_ = true; 
        if(!isInLoopThread())
            wakeup();
    }

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
    static EventLoop* getEventLoopOfCurrentThread();

    //因为使用了前向声明，没法做成内敛函数
    void addChannel(Channel& channel);
    void updateChannel(Channel& channel);
    void removeChannel(Channel& channel);

    void addTimer(std::function<void()> cb, int timeout, int interval = 0); //定时单位：毫秒

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
private:
    using ChannlList = std::vector<Channel*>;

    void abortNotInLoopThread();
    void wakeup();
    void handleRead();
    void doPendingFunctors();

    const pid_t threadId_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_; 
    bool looping_;
    bool quit_;
    bool isdoPendingFunctors;

    std::unique_ptr<Epoll> epoller_;
    std::unique_ptr<TimerQueue> timerQueue_;  //通用定时器队列
    
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;   //线程待处理事件,因为其他任何线程都可能委托事件给本线程，该变量需要靠mutex_同步
    ChannlList activeChannels_;
    
};

