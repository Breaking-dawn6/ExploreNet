#pragma once

#include "TcpServer.h"
#include "HttpRouter.h"
#include "HttpResponse.h"
#include "HttpContext.h"

#include <memory>
#include <functional>

using Task = std::function<void()>;
using TaskExecutor = std::function<void(Task)>;
using SyncHttpHandler = std::function<void(HttpRequest, HttpResponse &)>;

class HttpServer
{
public:
    HttpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg = "HttpServer");
    // ~HttpServer();

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    void start() { server_.start(); }

    void GET(const std::string &url, HttpHandler handler) { router_.GET(url, handler); }

    void SyncGet(const std::string &url, SyncHttpHandler handler);

    void POST(const std::string &url, HttpHandler handler) { router_.POST(url, handler); }

    void SyncPost(const std::string &url, SyncHttpHandler handler);

    void setDefaultHandler(HttpHandler handler) { router_.setDefaultHandler(std::move(handler)); }

    void setExecutor(TaskExecutor executor) { executor_ = std::move(executor); }

    HttpRouter &getRouter() { return router_; }

private:
    TcpServer server_;
    std::string name_;
    HttpRouter router_;
    TaskExecutor executor_;

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
};
