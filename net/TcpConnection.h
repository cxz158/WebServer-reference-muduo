/******************************************************************
*File Name: TcpConnection.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:40:22 AM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include "Buffer.h"
#include "TcpServer.h"
#include <memory>
#include <functional>
#include <unistd.h>
#include <arpa/inet.h>

class EventLoop;
class Channel;

class TcpConnection : noncopyable, std::enable_shared_from_this<TcpConnection>
{
public:
    using ConnectionCallback = TcpServer::ConnectionCallback;
    using MessageCallback = TcpServer::MessageCallback;
    using CloseCallback = TcpServer::CloseCallback;

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const struct sockaddr_in& perrAddr);
    ~TcpConnection() { close(sockfd_); }
    
    void setConnectionCallback(ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(MessageCallback& cb){ messageCallback_ = cb; }
    void setCloseCallback(CloseCallback& cb){ closecallback_ = cb; }
    std::string name() { return name_; }
private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead();
    void handleClose();
    void handleError();
    /* void connectDestroyed(); */

    EventLoop* loop_;
    std::string name_;
    StateE state_;
    int sockfd_;
    std::unique_ptr<Channel> channel_;
    struct sockaddr_in localAddr_;
    struct sockaddr_in peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closecallback_;

    Buffer outbuffer;
    Buffer inbuffer;
};

