#include "HttpServer.h"

HttpServer::HttpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg)
    : server_(loop, listenAddr, nameArg),
      name_(nameArg)
{
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                  { onConnection(conn); });
    server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                               { onMessage(conn, buf, time); });
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        // NOTE:最初，这里使用make_any的方式构造一个解析器
        // 但经过一系列的出错与排查， 发现使用make_any会导致llhttp_t内的data指向混乱（构造时指向this,但是被any拷贝之后依然指向原地址，因此会造成新的对象永远不更新状态）
        // 因此现在改为智能指针，同时删除了Context的拷贝构造与赋值方法
        conn->setContext(std::make_shared<HttpContext>());
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    std::any &anyContext = conn->getMutableContext();
    std::shared_ptr<HttpContext> context = std::any_cast<std::shared_ptr<HttpContext>>(anyContext);

    const char *data = buf->peek();
    size_t len = buf->readableBytes();

    if (!context->parse(data, len))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
        return;
    }

    buf->retrieve(len);

    if (context->isComplete())
    {
        if (requestAcceptor_)
            requestAcceptor_(conn, std::move(context->request()));
        context.reset();
    }
}
