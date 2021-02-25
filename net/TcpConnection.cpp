/******************************************************************
*File Name: /home/cxz/code/mywork/net/TcpConnection.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:59:06 AM PST
***************************************************************/
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "../base/log.h"
#include <assert.h>

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const struct sockaddr_in& perrAddr)
    : loop_(loop),
      name_(name),
      sockfd_(sockfd),
      channel_(new Channel(loop_,sockfd_)),
      peerAddr_(perrAddr)
{ 
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
}

void TcpConnection::handleRead()
{
    int n = inbuffer.readfd(sockfd_);
    if( n > 0 )
    {
       /* messageCallback_(shared_from_this(),); */
    }
    else if(n == 0){
        handleClose();
    }
    else{
        handleError();
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    log("TcpConnection::handleClose [%s]\n",name_.c_str());
    loop_->removeChannel(*channel_);
    closecallback_(shared_from_this());
}


void TcpConnection::handleError()
{
   loop_->assertInLoopThread();
   log_syserr("TcpConnection::handleError [%s]\n",name_.c_str());
}

/* void TcpConnection::connectDestroyed() */
/* { */
/*     loop_->assertInLoopThread(); */
/*     assert(state_ = kConnected); */
/*     setState(kDisconnected); */
/*     closecallback_(shared_from_this()); */
/*     loop_->removeChannel(*channel_); */
/* } */
