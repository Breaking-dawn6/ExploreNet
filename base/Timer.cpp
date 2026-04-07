#include "Timer.h"
#include "Timestamp.h"
#include "EventLoop.h"
#include "Logger.h"

#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <vector>

std::atomic<uint64_t> Timer::genId_{0};

TimerNodeId::TimerNodeId(Timestamp expire, uint64_t id)
    : expire_(expire),
      id_(id)
{
}

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

Timer::Timer(EventLoop *loop)
    : loop_(loop),
      timerfd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      timerfdChannel_(loop_, timerfd_),
      isCallingExpiredTimers_(false),
      timeQueue_(),
      cancelingTimers_()
{
    if (timerfd_ < 0)
    {
        LOG_FATAL("Failed in timerfd_create");
    }

    timerfdChannel_.setReadCallback([this](Timestamp receiveTime)
                                    { handleRead(); });
    timerfdChannel_.enableReading();
}

Timer::~Timer()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);

    for (TimerNode *node : timeQueue_)
    {
        delete node;
    }
}

TimerNodeId Timer::addTimer(Timestamp time, TimerCallback cb, double interval)
{
    Timestamp expire = time;
    TimerNode *node = new TimerNode(expire, GenId(), cb, interval);

    if (loop_->isInLoopThread())
    {
        addTimerInLoop(node);
    }
    else
    {
        loop_->runInLoop([this, node]()
                         { addTimerInLoop(node); });
    }

    return TimerNodeId(node->expiration(), node->id());
}

void Timer::addTimerInLoop(TimerNode *node)
{
    bool isEarliest = insert(node);

    if (isEarliest)
    {
        resetTimerfd(timerfd_, node->expiration());
    }
}

bool Timer::insert(TimerNode *node)
{
    bool isEarliest = false;
    Timestamp newNodeExpiration = node->expiration();
    auto it = timeQueue_.begin();
    if (it == timeQueue_.end() || newNodeExpiration < (*it)->expiration())
    {
        isEarliest = true;
    }
    timeQueue_.insert(node);
    return isEarliest;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    itimerspec newValue;
    itimerspec oldValue;
    memset(&newValue, 0, sizeof(struct itimerspec));
    memset(&oldValue, 0, sizeof(struct itimerspec));

    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_FATAL("timerfd_settime()");
    }
}

timespec howMuchTimeFromNow(Timestamp time)
{
    int64_t microseconds = time.MicroSecondsSinceEpoch() - Timestamp::systemRunTime().MicroSecondsSinceEpoch();

    if (microseconds < 100)
    {
        microseconds = 100;
    }
    timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);

    return ts;
}

void Timer::delTimer(TimerNodeId nodeId)
{
    if (loop_->isInLoopThread())
    {
        delTimerInLoop(nodeId);
    }
    else
    {
        loop_->runInLoop([this, nodeId]()
                         { delTimerInLoop(nodeId); });
    }
}

void Timer::delTimerInLoop(TimerNodeId nodeId)
{
    auto it = timeQueue_.find(nodeId);
    if (it != timeQueue_.end())
    {
        TimerNode *node = *it;
        timeQueue_.erase(node);
        delete node;
    }
    else if (isCallingExpiredTimers_)
    {
        cancelingTimers_.insert(nodeId);
    }
}

void Timer::handleRead()
{
    Timestamp now(Timestamp::systemRunTime());
    readTimerfd(timerfd_, now);
    std::vector<TimerNode *> expired = getExpired(now);

    isCallingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (const TimerNode *it : expired)
    {
        it->run();
    }
    isCallingExpiredTimers_ = false;

    reset(expired, now);
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t msgLength;
    ssize_t n = ::read(timerfd, &msgLength, sizeof(msgLength));
    LOG_INFO("TimerQueue::handleRead() %lu at %s", msgLength, now.toString().c_str());
    if (n != sizeof(msgLength))
    {
        LOG_ERROR("TimerQueue::handleRead() reads %ld bytes instead of 8", n);
    }
}

std::vector<TimerNode *> Timer::getExpired(Timestamp now)
{
    std::vector<TimerNode *> expired;

    TimerNodeId currentNode(now, UINT64_MAX);
    auto end = timeQueue_.lower_bound(currentNode);

    std::copy(timeQueue_.begin(), end, std::back_inserter(expired));
    timeQueue_.erase(timeQueue_.begin(), end);

    return expired;
}

void Timer::reset(const std::vector<TimerNode *> &expired, Timestamp now)
{
    Timestamp nextExpire;

    for (TimerNode *it : expired)
    {
        if (it->isRepeat() && cancelingTimers_.find(TimerNodeId(it->expiration(), it->id())) == cancelingTimers_.end())
        {
            it->restart(now);
            insert(it);
        }
        else
        {
            delete it;
        }
    }

    if (!timeQueue_.empty())
    {
        nextExpire = (*timeQueue_.begin())->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}
