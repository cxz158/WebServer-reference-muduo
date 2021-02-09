#include "Thread.h"
#include "CurrentThread.h"
#include <assert.h>
#include <iostream>


//缓存有关线程tid的信息，避免多次陷入内核
namespace CurrentThread
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "default";
}

void CurrentThread::cacheTid() 
{
  if (t_cachedTid == 0) 
  {
    t_cachedTid = gettid();
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}
//传递给线程的参数
struct ThreadData
{
    using ThreadFunc = Thread::ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* ptid_;
    CountDownLatch* platch_;

    ThreadData(ThreadFunc func, std::string name, pid_t* ptid,CountDownLatch* platch)
        :func_(func), name_(name), ptid_(ptid), platch_(platch){}

    void run_in_thread()
    {
        *ptid_ = CurrentThread::tid();
        ptid_ = nullptr;
        platch_->countDown();
        platch_ = nullptr;
        
        func_();
        std::cout<<"thread finished"<<std::endl;
        CurrentThread::t_threadName = "finished";
    }
};

//子线程启动函数
static void* start_thread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*> (obj);
    data->run_in_thread();
    delete data;
    return nullptr;
}

Thread::Thread(const ThreadFunc& func, const std::string& name)
    :pthreadId(0), tid_(0), name_(name), func_(func),
    latch_(1), started_(false), joined_(false)
{ }

void Thread::start()
{
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_,name_,&tid_,&latch_);
    if(pthread_create(&pthreadId, nullptr, start_thread,data) == 0)
    {
        std::cout<<"thread "<<pthreadId<<" created"<<std::endl;
        latch_.wait(); //等待子线程设置tid
        assert(tid_ > 0);
    } else {
        started_ = false; // 创建子线程失败
        delete data;
    }
}

int Thread::join()
{
   assert(started_);
   assert(!joined_);
   joined_ = true;
   std::cout<<"thread "<<pthreadId<<" joined"<<std::endl;
   return pthread_join(pthreadId, nullptr);
}

Thread::~Thread()
{
    if(started_ && !joined_)
        pthread_detach(pthreadId);
}
