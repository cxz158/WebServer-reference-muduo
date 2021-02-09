#pragma once
#include "Mutex.h"
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

//条件变量 pthread_mutex_t + pthread_cond_t
class Condition
{
public:
    Condition(MutexLock& mutex):mutex_(mutex){pthread_cond_init(&cond_,NULL);}
    void wait(){pthread_cond_wait(&cond_,&mutex_.get_pthread_mutex());}
    void notify_one(){pthread_cond_signal(&cond_);}
    void notify_all(){pthread_cond_broadcast(&cond_);}
    bool wait_seconds(int seconds) { 
        struct timeval tv;
        struct timespec ts;
        
        if(gettimeofday(&tv,NULL))
            return false;
        ts.tv_sec = tv.tv_sec + static_cast<time_t>(seconds);
        ts.tv_nsec = tv.tv_usec * 1000;
        return ETIMEDOUT == pthread_cond_timedwait(&cond_,&mutex_.get_pthread_mutex(),&ts);
        
    }
    ~Condition(){pthread_cond_destroy(&cond_);}
private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};

