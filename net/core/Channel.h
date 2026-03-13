#pragma once

#include "nocopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/*
一个Thread包含一个EventLoop,一个EventLoop拥有一个Poller,一个Poller监听多个Channel,而一个Channel管理一个fd监听的事件并负责调起回调函数
Channel类似于一个通道，封装了sockfd与其关心的event，如EPOLLIN、EPOLLOUT
还绑定了poller返回的具体事件
*/

class Channel
{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd得到poller通知后，处理事件的
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallBack(ReadEventCallBack cb) { readCallBack_ = std::move(cb); }
    void setWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }
    void setErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

    // 防止channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    int set_events(int revt) { return revents_ = revt; }

    // 设置fd相应的事件状态
    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWritting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd,poller监听的对象
    int events_;      // 注册fd感兴趣的对象
    int revents_;     // poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channel通道内能获知fd最终发生的具体事件的revents，所以它负责调用具体事件的回调操作
    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};