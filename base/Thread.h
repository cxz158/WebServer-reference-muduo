#pragma once
#include <pthread.h>
#include <string>
#include <functional>
#include <unistd.h>
#include "noncopyable.h"
#include "CountDownLatch.h"

extern const std::string defalut_thread_name;

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(const ThreadFunc&, const std::string& name = defalut_thread_name);
    void start();
    int join();
    bool started() const {return started_;}
    pid_t tid() const {return tid_;}
    const std::string name() const {return name_;}
    ~Thread();

private:
    pthread_t pthreadId;
    pid_t tid_;
    std::string name_;    
    ThreadFunc func_;
    CountDownLatch latch_;
    bool started_;
    bool joined_;
};

