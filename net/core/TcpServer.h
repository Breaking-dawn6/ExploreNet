#pragma once

// 考虑到这是用户使用的类，故直接包含项目可能用到的类以方便用户使用而不使用前向声明
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "nocopyable.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "TcpConnection.h"
#include "Buffer.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

// 对外的服务器编程使用的类
class TcpServer : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option = kNoReusePort);
    ~TcpServer();

    // note:自行优化
    void setThreadInitCallback(const ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); }
    void setConnectionCallback(const ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(const MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(const WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

    // 设置底层subLoop个数
    void setThreadNum(int numThreads);

    // 开启服务器监听
    void start();

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_; // baseLoop,即需要用户自己定义的loop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;              // 运行在mainLoop,主要任务就是监听新连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成时的回调

    ThreadInitCallback threadInitCallback_; // loop线程初始化完成时的回调
    std::atomic_int started_;

    int nextConnId_;

    ConnectionMap connections_; // 保存所有的连接
};