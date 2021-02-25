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


TcpServer::TcpServer(EventLoop* loop, int port, std::string name)
    : loop_(loop),
      name_(name),
      nextId_(0),
      acceptor_(new Acceptor(loop_, port))
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));    
}

void TcpServer::newConnection(int connfd, const struct sockaddr_in& perrAddr)
{
    loop_->assertInLoopThread();
    std::string connName = name_ + format_string("#%d",nextId_++);

    log("TcpServer::newConnection [%s], - new connection [%s] from %s",
        name_.c_str(), connName.c_str(), sock_ntop_ipv4(perrAddr).c_str());

    TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, connName, connfd, perrAddr);
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConn, this, std::placeholders::_1));
}

void TcpServer::removeConn(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    connections_.erase(conn->name()); //因为conn 的存在，erase后 TcpConnection 依然还未销毁 
    
}
