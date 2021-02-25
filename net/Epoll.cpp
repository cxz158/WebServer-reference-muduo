#include "Epoll.h"
#include "Channel.h"
#include "../base/log.h"
#include <assert.h>

static const int NUMEVENTS = 16;

Epoll::Epoll(EventLoop* loop):
    epollfd_(epoll_create1(EPOLL_CLOEXEC)),
    ownerLoop_(loop),
    events_(NUMEVENTS)
{
    assert(epollfd_ > 0);
}

void Epoll::poll(ChannelList& activeChannels, int time)
{
    
    int numEvents = epoll_wait(epollfd_, events_.data(), events_.size(), time);
    int savedErrno = errno;
    if(numEvents >= 0)
    {
        log("epoll_wait get %d events\n", numEvents);
        for(int i = 0; i < numEvents; ++i)
        {
            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->set_revents(events_[i].events);
            activeChannels.push_back(channel);
        }

        if(static_cast<size_t>(numEvents) == events_.size())  //事件过多，扩大一次poll允许唤醒的io数量
        {
            events_.resize(events_.size()*2);
        }
    }
    else{
        errno = savedErrno;
        log_syserr("Epoll::poll()\n");
    }   
}

void Epoll::channel_xxx(Channel& channel, int operation)
{
    struct epoll_event event;
    int fd = channel.fd();
    event.events = channel.events();
    event.data.ptr = &channel;
    if(epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        log_syserr("call epoll_ctl error! this fd = %d, operation = %s\n", fd, operationToString(operation));
    }
}


std::string Epoll::operationToString(int operation)
{
    switch(operation)
    {
        case EPOLL_CTL_ADD : return "ADD";
        case EPOLL_CTL_MOD : return "MOD";
        case EPOLL_CTL_DEL : return "DEL";

        default: return "UNKONW OPERATION";
    }
}
