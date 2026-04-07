#pragma once

#include <set>
#include <atomic>

#include "Callbacks.h"
#include "Channel.h"

class EventLoop;

struct TimerNodeId
{
    Timestamp expire_;
    uint64_t id_;
    TimerNodeId(Timestamp expire, uint64_t id);
};

struct TimerNode : protected TimerNodeId
{

public:
    TimerNode(Timestamp expire, uint64_t id, TimerCallback cb, double interval = 0.0);
    void run() const
    {
        timerCallback_();
    }

    void restart(Timestamp now);

    Timestamp expiration() const { return expire_; }
    uint64_t id() const { return id_; }
    bool isRepeat() const { return isRepeat_; }
    double interval() const { return interval_; }

private:
    TimerCallback timerCallback_;
    bool isRepeat_;
    double interval_;
};

struct TimerNodeComparator
{
    // 启动异构查找，方便使用TimerNodeId来在timeQueue中查找
    using is_transparent = void;

    bool operator()(const TimerNode *lhs, const TimerNode *rhs) const
    {
        return compareImpl(lhs->expiration(), lhs->id(),
                           rhs->expiration(), rhs->id());
    }

    bool operator()(const TimerNode *lhs, const TimerNodeId &rhs) const
    {
        return compareImpl(lhs->expiration(), lhs->id(),
                           rhs.expire_, rhs.id_);
    }

    bool operator()(const TimerNodeId &lhs, const TimerNode *rhs) const
    {
        return compareImpl(lhs.expire_, lhs.id_,
                           rhs->expiration(), rhs->id());
    }

private:
    bool compareImpl(Timestamp lhsTime, uint64_t lhsId,
                     Timestamp rhsTime, uint64_t rhsId) const;
};

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
    std::atomic<bool> isCallingExpiredTimers_;
    std::set<TimerNodeId> cancelingTimers_;
    EventLoop *loop_;
    int timerfd_;
    Channel timerfdChannel_;
    std::set<TimerNode *, TimerNodeComparator> timeQueue_;
};

void readTimerfd(int timerfd, Timestamp now);
void resetTimerfd(int timerfd, Timestamp expiration);
timespec howMuchTimeFromNow(Timestamp time);
