#include "CurrentThread.h"
#include <unistd.h>
#include <stdio.h>

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
