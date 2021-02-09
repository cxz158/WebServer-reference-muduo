#pragma once
#include <sys/types.h>
#include "../base/CurrentThread.h"

class EventLoop
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
    EventLoop* getEventLoopofCurrentThread();
private:
    void abortNotInLoopThread();

    const pid_t threadId_;
    bool looping_;
};

