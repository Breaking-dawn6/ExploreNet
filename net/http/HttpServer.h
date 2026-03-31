#pragma once

#include "TcpServer.h"
#include "HttpContext.h"
#include "HttpResponse.h"
#include "HttpRouter.h"

#include <memory>

class HttpServer
{
public:
    HttpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg = "HttpServer");
    // ~HttpServer();

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    void start() { server_.start(); }

    void GET(const std::string &url, HttpHandler handler) { router_.GET(url, handler); }

    void POST(const std::string &url, HttpHandler handler) { router_.POST(url, handler); }

    void setDefaultHandler(HttpHandler handler) { router_.setDefaultHandler(std::move(handler)); }

    HttpRouter &getRouter() { return router_; }

private:
    TcpServer server_;
    std::string name_;
    HttpRouter router_;

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
};
