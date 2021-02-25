/******************************************************************
*File Name: TcpSever.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:06:58 AM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include <arpa/inet.h>
#include <functional>
#include <memory>
#include <map>

class EventLoop;
class Acceptor;
class TcpConnection;

class TcpServer : noncopyable
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void()>;
    using MessageCallback = std::function<void(const char*)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpServer(EventLoop* loop, int port, std::string name = "TcpServer");
    ~TcpServer();

    //这两Function将传递给TcpConnection
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    void newConnection(int connfd, const sockaddr_in& perrAddr);
    void removeConn(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string name_;
    int nextId_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionMap connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};

