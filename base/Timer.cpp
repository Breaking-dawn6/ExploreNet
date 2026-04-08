#include "Timer.h"
#include "Timestamp.h"
#include "EventLoop.h"
#include "Logger.h"

#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <vector>

std::atomic<uint64_t> Timer::genId_{0};

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
    // 用于标记此次插入的定时器是否是队列中最早触发的
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
    // 若队列中没有定时器，或者新插入的定时器到期时间小于队列中最早的定时器
    if (it == timeQueue_.end() || newNodeExpiration < (*it)->expiration())
    {
        isEarliest = true;
    }
    timeQueue_.insert(node);
    return isEarliest;
}

// 重新设置timerfd的触发时间
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
        // 若正在执行定时器任务则将待删除的定时器加入队列
        cancelingTimers_.insert(nodeId);
    }
}

void Timer::handleRead()
{
    Timestamp now(Timestamp::systemRunTime());
    readTimerfd(timerfd_, now);
    std::vector<TimerNode *> expired = getExpired(now);

    cancelingTimers_.clear();
    isCallingExpiredTimers_ = true;
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

// 从定时器列表中移出到期的定时器
std::vector<TimerNode *> Timer::getExpired(Timestamp now)
{
    std::vector<TimerNode *> expired;

    TimerNodeId currentNode(now, UINT64_MAX);
    auto end = timeQueue_.lower_bound(currentNode);

    std::copy(timeQueue_.begin(), end, std::back_inserter(expired));
    timeQueue_.erase(timeQueue_.begin(), end);

    return expired;
}

// 删除到期的非重复定时器
void Timer::reset(const std::vector<TimerNode *> &expired, Timestamp now)
{
    Timestamp nextExpire;

    for (TimerNode *it : expired)
    {
        // 若该定时器为重复定时器且未被用户指定删除则将其重新加入队列
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
