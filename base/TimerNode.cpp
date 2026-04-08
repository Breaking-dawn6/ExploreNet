#include "TimerNode.h"

TimerNodeId::TimerNodeId(Timestamp expire, uint64_t id)
    : expire_(expire),
      id_(id)
{
}

// 默认构造一个无效的定时器索引
TimerNodeId::TimerNodeId()
    : expire_(Timestamp::invalid()),
      id_(UINT64_MAX)
{
}

// 按时间先后对比，若同时触发则按插入顺序对比
bool operator<(const TimerNodeId &left, const TimerNodeId &right)
{
    if (left.expire_ < right.expire_)
        return true;
    else if (left.expire_ == right.expire_ && left.id_ < right.id_)
        return true;
    return false;
}

TimerNode::TimerNode(Timestamp expire, uint64_t id, TimerCallback cb, double interval)
    : TimerNodeId(expire, id),
      interval_(interval),
      timerCallback_(std::move(cb))
{
    isRepeat_ = interval_ > 0.0;
}

// 为重复定时器设置下一次触发时间
void TimerNode::restart(Timestamp now)
{
    if (isRepeat_)
    {
        expire_ = now + interval_;
    }
    else
    {
        expire_ = Timestamp::invalid();
    }
}

bool TimerNodeComparator::compareImpl(Timestamp lhsTime, uint64_t lhsId,
                                      Timestamp rhsTime, uint64_t rhsId) const
{
    if (lhsTime < rhsTime)
        return true;
    else if (lhsTime == rhsTime && lhsId < rhsId)
        return true;
    return false;
}