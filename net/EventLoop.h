#pragma once
#include <sys/types.h>
#include "../base/CurrentThread.h"
#include "../base/noncopyable.h"

//一个线程只允许有一个EventLoop , 感觉有点像线程版的单例模式
class EventLoop: noncopyable
{
public:
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

    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}
    static EventLoop* getEventLoopofCurrentThread();
private:
    void abortNotInLoopThread();

    const pid_t threadId_;
    bool looping_;
};

