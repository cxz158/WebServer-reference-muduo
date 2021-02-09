#pragma once
#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    ~CountDownLatch() {}

private:
    MutexLock mutex_;
    Condition condition_;
    int count_;
};

