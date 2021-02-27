/******************************************************************
*File Name: TcpSever.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:06:58 AM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include <arpa/inet.h>
#include <functional>
#include <memory>
#include <map>

class EventLoop;
class Acceptor;

class TcpServer : noncopyable
{
public:
    using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;
    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using MessageCallback = TcpConnection::MessageCallback;
    using CloseCallback = TcpConnection::CloseCallback;

    TcpServer(EventLoop* loop, int port, int threadNum,  std::string name = "TcpServer");
    ~TcpServer(){};

    //这两Function将传递给TcpConnection
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
 
    void newConnection(int connfd, const sockaddr_in& localAddr, const sockaddr_in& peerAddr);
    void removeConn(const TcpConnectionPtr& conn);
    void removeConnInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string name_;
    int nextId_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionMap connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
};

