#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI | EPOLLHUP | EPOLLET;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revnets_(0),
      eventHandling_(false)
{  }



void Channel::handleEvent()
{
    eventHandling_ = true;
    auto guard = tie_.lock();
    if(revnets_ & EPOLLERR)
        if(errorCallback_) errorCallback_();
    if(revnets_ & kReadEvent)
        if(readCallback_) readCallback_();
    if(revnets_ & kWriteEvent)
        if(writeCallback_) writeCallback_();
    if((revnets_ & EPOLLHUP) && !(revnets_ & EPOLLIN))
        if(closeCallback_) closeCallback_();
    eventHandling_ = false;
}
