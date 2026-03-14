#include "TcpServer.h"
#include "Logger.h"

#include <functional>
#include <strings.h>

EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(),
      messageCallback_(),
      nextConnId_(1),
      started_(0)
{
    // 当有新用户连接时会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback([this](int sockfd, const InetAddress &peerAddr)
                                        { newConnection(sockfd, peerAddr); });
}

TcpServer::~TcpServer()
{
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second); // 局部强智能指针会在出作用域后自动释放TcpConnection
        item.second.reset();

        // 销毁连接
        conn->getLoop()->runInLoop([conn]()
                                   { conn->connectDestroyed(); });
    }
}

// 开启服务器监听    loop.loop()
void TcpServer::start()
{
    if (started_++ == 0) // 防止一个tcpSever对象被start多次
    {
        threadPool_->start(threadInitCallback_); // 启动底层线程池
        loop_->runInLoop([this]()
                         { acceptor_.get()->listen(); });
    }
}

// 有一个新的客户端的连接，acceptor会执行那个这个回调
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 轮询算法，选择一个subLoop来管理channel
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};

    // 连接名称
    snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);

    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
             name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }

    InetAddress localAddr(local);

    // 根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd, // socket channel
        localAddr,
        peerAddr));

    connections_[connName] = conn;

    // 下面的回调是由用户所设置给TcpServer -> TcpConnection -> Channel -> Poller -> notify channel使用回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    // 设置了如何关闭连接的回调 conn -> shutdown
    conn->setCloseCallback([this](const TcpConnectionPtr &conn)
                           { removeConnection(conn); });

    // 直接调用TcpConnection::connectEstablished
    ioLoop->runInLoop([conn]()
                      { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop([this, conn]()
                     { removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s", name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());

    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn]()
                        { conn->connectDestroyed(); });
}

void TcpServer::setThreadNum(int numThreads)
{
    if (numThreads < 0)
    {
        LOG_WARN("numThreads must be greater than or equal to 0 , It has now been reset to 0.");
    }
    threadPool_->setThreadNum(std::max(0, numThreads));
}