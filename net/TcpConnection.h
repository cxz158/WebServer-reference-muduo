/******************************************************************
*File Name: TcpConnection.h
*Author: cxz158
*mail: chenxiangzhon17@qq.com
*Created Time: Mon 22 Feb 2021 03:40:22 AM PST
***************************************************************/
#pragma once
#include "../base/noncopyable.h"
#include "Buffer.h"
#include <memory>
#include <functional>
#include <unistd.h>
#include <arpa/inet.h>

class EventLoop;
class Channel;

class TcpConnection : noncopyable,      //enable_shared_from_this() 必要要public继承!
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void()>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const struct sockaddr_in& localAddr, const struct sockaddr_in& peerAddr);
    ~TcpConnection() { close(sockfd_); }
    
    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb; }
    //for TcpServer
    void setCloseCallback(const CloseCallback& cb){ closecallback_ = cb; }

    void send(const std::string& message);
    struct sockaddr_in getPeerAddr(){ return peerAddr_; }
    void shutdown();
    std::string name() { return name_; }
    EventLoop* getLoop() { return loop_; }
    void connectDestroyed();
    void connectEstablished();
private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

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

