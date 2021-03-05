#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"

//因为在本程序模型中，socktfd 被分配到一个eventloop之后就一直由该线程
//处理socktfd上接受的数据，即没有设置专门的threadpool处理数据，能够保证
//接受到的数据不会被两个线程争用，因此无需设置EPOLLONESHOT
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLET; 
const int Channel::kWriteEvent = EPOLLOUT | EPOLLET;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revnets_(0),
      eventHandling_(false),
      tied(false)
{  }



void Channel::handleEvent()
{
    std::shared_ptr<void> guard;
    if(tied)
    {
        guard = tie_.lock();
        if(guard)
            handleEventWithGuard();
    } else {
        handleEventWithGuard();
    }
}

void Channel::handleEventWithGuard()
{
    eventHandling_ = true;
    if(revnets_ & EPOLLOUT)
        if(writeCallback_) writeCallback_();
    if(revnets_ & (EPOLLIN | EPOLLHUP | EPOLLPRI))
        if(readCallback_) readCallback_();
    eventHandling_ = false;
}
