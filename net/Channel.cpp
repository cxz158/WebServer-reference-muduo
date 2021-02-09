#include "Channel.h"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      events_(0),
      revnets_(0),
      index_(-1)
{  }

void Channel::update()
{
    
}

void Channel::handleEvent()
{
    if(revnets_ & (EPOLLERR))
        if(errorCallback_) errorCallback_();
    if(revnets_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        if(readCallback_) readCallback_();
    if(revnets_ & EPOLLOUT)
        if(writeCallback_) writeCallback_();
}
