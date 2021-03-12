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
#include "../base/CurrentThread.h"
#include <assert.h>
#include <error.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

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
    /* log("[%s] accept conn [%s]\n", CurrentThread::name(), name_.c_str()); */
    channel_->tie(shared_from_this());
    channel_->enableReading();
    channel_->enableWriting();
    loop_->addChannel(*channel_);
}

TcpConnection::~TcpConnection()
{
    assert(state_ == kDisconnected);
    /* log("[%s] TcpConnection destory\n",name_.c_str()); */
    close(sockfd_);
}

void TcpConnection::send(const char* message, size_t len)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message, len);
        }
        else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message, len));
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
    bool zero;
    zero = inbuffer.readfd(sockfd_); //readfd() 缓冲区足够大，能够保证一次读完，无需循环调用
    if(!zero)
        messageCallback_(shared_from_this(), &inbuffer);
    else
        handleClose();
}


//event = EPOLLOUT | EPOLLET
void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(outbuffer.readableBytes() > 0)
    {
        size_t n = write(sockfd_, outbuffer.readbegin(), outbuffer.readableBytes());
        if(n > 0)
        {
            outbuffer.retrive(n);
            if(outbuffer.readableBytes() == 0) //缓冲区buffer 中可读数据为0，取消对EPOLLOUT 事件的关注
            {
                outbuffer.init();
                log("[%s]message send complete\n",name_.c_str());
                if(state_ == kDisconnecting)  //如果打算关闭，就可以shutdown了接下来就交给TCP协议栈
                    shutdownInLoop();
            }else{
                log("[%s]I am going to write more data\n",name_.c_str());
            }        
        }else{
            log_syserr("TcpConnection::handleWrite error!\n");
        }
    }
}


void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    loop_->removeChannel(*channel_);
    /* log("[%s] TcpConnection::handleClose [%s]\n",CurrentThread::name(), name_.c_str()); */
    TcpConnectionPtr guard(shared_from_this());
    connectionCallback_(guard);
    closecallback_(guard);
}


void TcpConnection::handleError()
{
    loop_->assertInLoopThread();
    log_syserr("TcpConnection::handleError [%s]\n",name_.c_str());
}

void TcpConnection::sendInLoop(const char* msg, size_t len)
{
    loop_->assertInLoopThread();
    size_t writen = 0;
    //缓冲区中暂无数据, 直接write
    if(outbuffer.readableBytes() == 0)
    {
        writen = write(sockfd_, msg, len);
        if(writen >= 0)
        {
            if(writen < len) //没写全
            {
                log("[%s] I am going to write more data\n", name_.c_str());
            }
        }else{
            writen = 0;
            log_syserr("[%s] TcpConnection::sendInLoop\n", name_.c_str());
        }
    }

    //实际写入的数据少于需要写入的数据，将还未写入的数据append到buffer中
    if(writen < len)
    {
        outbuffer.append(msg + writen, len-writen);
        handleWrite();
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(outbuffer.readableBytes() == 0)    //等buffer 里的数据写完
    {
        ::shutdown(sockfd_, SHUT_WR); //wo will don't write
    }
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        loop_->removeChannel(*channel_);
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::forceClose()
{
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    setState(kDisconnecting);
    loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting)
  {
    handleClose();
  }
}
