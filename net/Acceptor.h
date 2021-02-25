/******************************************************************
*File Name: Acceptor.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 03:54:42 AM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include "Channel.h"
#include <functional>
#include <unistd.h>
#include <sys/socket.h>

class EventLoop;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const struct sockaddr_in&)>;

    Acceptor(EventLoop* loop, int port = 8080);
    ~Acceptor() { close(sockfd_); }

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
private:
    void handleRead();

    static int MAXFDNUM;

    EventLoop* loop_;
    int sockfd_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
};

