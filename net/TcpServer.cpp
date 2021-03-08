/******************************************************************
*File Name: /home/cxz/code/mywork/net/TcpSever.cpp
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:22:31 AM PST
***************************************************************/
#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "SocketOps.h"
#include "../base/log.h"
#include <memory>
#include <functional>
#include <assert.h>


TcpServer::TcpServer(EventLoop* loop, int ThreadNum, int port, std::string name)
    : loop_(loop),
      name_(name),
      nextId_(0),
      acceptor_(new Acceptor(loop_, port)),
      threadPool_(new EventLoopThreadPool(loop_, ThreadNum))
{
    ignore_sig_pipe();
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));    
}

void TcpServer::newConnection(int connfd, const struct sockaddr_in& localAddr, const struct sockaddr_in& peerAddr)
{
    loop_->assertInLoopThread();
    std::string connName = name_ + format_string(" conn#%d",nextId_++); log("%sTcpServer::newConnection [%s] - new connection [%s] from %s\n", get_time().c_str(),  name_.c_str(), connName.c_str(), sock_ntop_ipv4(peerAddr).c_str());

    EventLoop* ioLoop = threadPool_->getNextLoop();
    auto conn = std::make_shared<TcpConnection>(ioLoop, connName, connfd, localAddr, peerAddr);
    connections_[connName] = conn;
    /* conn->setConnectionCallback(connectionCallback_); */
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConn, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));  
}

void TcpServer::removeConn(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&::TcpServer::removeConnInLoop, this, conn));
}

void TcpServer::removeConnInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    int ret = connections_.erase(conn->name());
    if(ret == 1)
    {
        EventLoop* ioLoop = conn->getLoop();
        ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn)); //conn 会存活到退出该函数。 
    }
}
