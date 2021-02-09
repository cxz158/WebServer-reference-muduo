#pragma once
#include <pthread.h>
#include "noncopyable.h"

class MutexLock:noncopyable
{
public:
    MutexLock() {pthread_mutex_init(&mutex_, NULL);}
    ~MutexLock() {pthread_mutex_destroy(&mutex_);}
private:
    friend class MutexLockGuard;
    friend class Condition;
    void lock(){pthread_mutex_lock(&mutex_);}
    void unlock(){pthread_mutex_unlock(&mutex_);}
    pthread_mutex_t& get_pthread_mutex(){return mutex_;}
private:
    pthread_mutex_t mutex_;
};


/* 
类的生存周期来保管锁,构造的时候lock(),析构的时候unlock()
使用方式:
{
...//临界区外
    {
        MutexLockGuard lock(mutex);
        ...//临界区代码
    }
...//临界区外
}
*/
class MutexLockGuard :noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex):mutex_(mutex){mutex_.lock();}
    ~MutexLockGuard(){mutex_.unlock();}
private:
    MutexLock& mutex_;
};

