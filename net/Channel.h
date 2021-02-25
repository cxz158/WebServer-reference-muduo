#pragma once
#include "../base/noncopyable.h"
#include <functional>
#include <memory>
#include <assert.h>

class EventLoop;

// A selectable I/O channel
// This class doesn't own the file descriptor
// the file descriptor could be a socket
// an eventfd, a timerfd, or a signalfd

class Channel: noncopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel(){ assert(!eventHandling_); };

    void handleEvent();
    void setReadCallback(const EventCallback& cb){ readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb){ writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb){ errorCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb){ closeCallback_ = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    
    void set_revents(int revt){ revnets_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; }
    void enableWriting() { events_  |= kWriteEvent; }
    //void disableAll() { events_ = kNoneEvent; }

    EventLoop* ownerLoop() { return loop_; }

private:

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revnets_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;

    bool eventHandling_;
};

using ChannelPtr = std::shared_ptr<Channel>;
