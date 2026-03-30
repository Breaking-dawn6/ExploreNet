#include "TcpServer.h"
#include "HttpResponse.h"
#include "HttpContext.h"

#include <memory>

using RequestCallback = std::function<void(const TcpConnectionPtr &, HttpRequest)>;

class HttpServer
{
public:
    HttpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg = "HttpServer");
    // ~HttpServer();

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    void start() { server_.start(); }

    void setRequestAcceptor(RequestCallback cb) { requestAcceptor_ = cb; }

private:
    TcpServer server_;
    std::string name_;
    RequestCallback requestAcceptor_;

    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
};
