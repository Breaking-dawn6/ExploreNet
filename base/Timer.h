#pragma once

#include <set>
#include <atomic>

#include "Channel.h"
#include "TimerNode.h"

class TimerNodeId;
class TimerNodeComparator;
class EventLoop;

class Timer
{
public:
    explicit Timer(EventLoop *loop);
    ~Timer();

    TimerNodeId addTimer(Timestamp time, TimerCallback cb, double interval = 0.0);
    void delTimer(TimerNodeId nodeId);

private:
    static uint64_t GenId()
    {
        return genId_++;
    }

    void addTimerInLoop(TimerNode *node);
    bool insert(TimerNode *node);
    void delTimerInLoop(TimerNodeId nodeId);

    void handleRead();

    void reset(const std::vector<TimerNode *> &expired, Timestamp now);

    std::vector<TimerNode *> getExpired(Timestamp now);

    static std::atomic<uint64_t> genId_;
    std::atomic<bool> isCallingExpiredTimers_; // 是否正在执行定时器
    std::set<TimerNodeId> cancelingTimers_;    // 待取消的定时器集合
    EventLoop *loop_;
    int timerfd_;
    Channel timerfdChannel_;
    std::set<TimerNode *, TimerNodeComparator> timeQueue_;
};

void readTimerfd(int timerfd, Timestamp now);
void resetTimerfd(int timerfd, Timestamp expiration);
timespec howMuchTimeFromNow(Timestamp time);
