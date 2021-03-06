/******************************************************************
*File Name: /home/cxz/code/mywork/net/Acceptor.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Sun 21 Feb 2021 04:57:00 AM PST
***************************************************************/
#include "../base/log.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketOps.h"
#include <sys/socket.h>
#include <string.h>
#include <functional>
#include <errno.h>

int Acceptor::MAXFDNUM = 30000;

Acceptor::Acceptor(EventLoop* loop, int port)
    : loop_(loop),
      localAddr(),
      sockfd_(socket_bind_listend_noblock(port, localAddr)),
      acceptChannel_(loop_, sockfd_),
      newConnectionCallback_(nullptr)
{
    acceptChannel_.enableReading();
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
    log("Acceptor: start listenning to port:%d\n",port);
    loop_->addChannel(acceptChannel_);
}

void Acceptor::handleRead()
{
    loop_->isInLoopThread();
    struct sockaddr_in peerAddr;
    bzero(&peerAddr, sizeof peerAddr);
    socklen_t perrAddrlength = sizeof peerAddr;
    int connfd; 
    while((connfd = accept4(sockfd_,
                            (struct sockaddr*)&peerAddr,
                            &perrAddrlength, SOCK_NONBLOCK | SOCK_CLOEXEC)) > 0)
    {
        if(newConnectionCallback_ && connfd < MAXFDNUM)  //避免文件描述符耗尽
        {
            newConnectionCallback_(connfd, localAddr, peerAddr);
        }
        else
        {
            log("Acceptor: Internal server busy file description will run out\n");
            close(connfd);
        }
    }
    if(errno != EAGAIN)
    {
        log_syserr("Acceptor: error!\n");
    }
}
