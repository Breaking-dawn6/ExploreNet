#pragma once

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;
class TimerNode;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ThreadInitCallback = std::function<void(EventLoop *)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using TimerCallback = std::function<void(void)>;

using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;

using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

class Callbacks
{
};