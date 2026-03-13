#include "Poller.h"
#include "EPollPoller.h"

#include <stdlib.h>

// 作为一个公共的实现文件，避免出现基类引用派生类头文件的现象
Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (getenv("MUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll的实例
    }
    else
    {
        return new EPollPoller(loop); // 生成epoll的实例
    }
}