#pragma once

#include "Timestamp.h"
#include "Callbacks.h"

// 用于最小化查找Timer
struct TimerNodeId
{
    TimerNodeId(Timestamp expire, uint64_t id);
    TimerNodeId();

    Timestamp expire_;
    uint64_t id_;
};

// 每个TimerNode对应一个定时器实体
class TimerNode : protected TimerNodeId
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

bool operator<(const TimerNodeId &left, const TimerNodeId &right);
