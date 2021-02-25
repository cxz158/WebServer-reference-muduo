#pragma once
#include "../base/noncopyable.h"
#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <memory>

class Channel;


class Epoll : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;


    Epoll(EventLoop* loop);
    ~Epoll(){ close(epollfd_); };
    
    void channel_add(Channel& channel){ channel_xxx(channel,EPOLL_CTL_ADD); } 
    void channel_mod(Channel& channel){ channel_xxx(channel,EPOLL_CTL_MOD); }
    void channel_del(Channel& channel){ channel_xxx(channel,EPOLL_CTL_DEL); }

    void poll(ChannelList& activeChannels, int time);

private:
    using EventList = std::vector<struct epoll_event>; 

    void channel_xxx(Channel& channel, int operation);
    std::string operationToString(int operation);
    
    int epollfd_;
    EventLoop* ownerLoop_;
    EventList events_;
};

