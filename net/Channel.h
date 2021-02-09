#pragma once
#include "../base/noncopyable.h"
#include <functional>

class EventLoop;


class Channel: noncopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();
    void setReadCallback(const EventCallback& cb){ readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb){ writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb){ errorCallback_ = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    
    void set_revents(int revt){ revnets_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_  |= kWriteEvent; update(); }

    int index(){return index_;}
    void set_index(int idx) {index_ = idx;}

    EventLoop* ownerLoop() { return loop_; }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revnets_;
    int index_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
};

