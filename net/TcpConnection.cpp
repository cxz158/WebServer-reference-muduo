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

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                             const struct sockaddr_in& localAddr, const struct sockaddr_in& perrAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      sockfd_(sockfd),
      channel_(new Channel(loop_,sockfd_)),
      localAddr_(localAddr),
      peerAddr_(perrAddr)
{ 
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    loop_->addChannel(*channel_);
}

void TcpConnection::send(const std::string& message)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::handleRead()
{
    int n = inbuffer.readfd(sockfd_); //readfd() 缓冲区足够大，能够保证一次读完，无需循环调用
    if( n > 0 )
    {
        messageCallback_(shared_from_this(), &inbuffer);
    }
    else if(n == 0){
        handleClose();
    }
    else{
        handleError();
    }
}

//event = EPOLLOUT | EPOLLET | EPOLLONESHOT
void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
        size_t n = write(sockfd_, outbuffer.peek(), outbuffer.readableBytes());
        if(n > 0)
        {
            outbuffer.retrive(n);
            if(outbuffer.readableBytes() == 0) //缓冲区buffer 中可读数据为0，取消对EPOLLOUT 事件的关注
            {
                channel_->disableWriting();
                loop_->updateChannel(*channel_);
                if(state_ == kDisconnecting)  //将buffer 中的数据全部发出，就可以shutdown了接下来就交给TCP协议栈
                {
                    shutdown();
                }
            }else{
                log("I am going to write more data\n");
            }        
        }else{
            log_syserr("TcpConnection::handleWrite\n");
        }
    }else{
        log("Connetion is down, no more writing\n");
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

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    size_t writen = 0;
    //还未向socket写入数据， 缓冲区中暂无数据, 直接write
    if(!channel_->isWriting() && outbuffer.readableBytes() == 0)
    {
        writen = write(sockfd_, message.data(), message.size());
        if(writen >= 0)
        {
            if(writen < message.size()) //没写全
            {
                log("I am going to write more data\n");
            }
        }
        else
        {
            log_error_fatal("TcpConnection::sendInLoop\n");
        }
    }

    //readableBytes() != 0
    //buffer数据没写完， append入buffer, 关注channel的写事件, 提醒还有数据要写入socket
    if(writen < message.size())
    {
        outbuffer.append(message.data()+writen, message.size()-writen);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();
            loop_->updateChannel(*channel_);
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())    //等buffer 里的数据写完
    {
        ::shutdown(sockfd_, SHUT_WR); //wo will don't write
    }
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_ = kConnected);
    setState(kDisconnected);
    loop_->removeChannel(*channel_);
}
