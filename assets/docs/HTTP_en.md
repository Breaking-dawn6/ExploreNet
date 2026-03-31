<center><a href="HTTP.md">简体中文</a> | <a href="HTTP_en.md">English</a></center>



HTTP support is built upon the llhttp open-source library.

llhttp project URL: https://github.com/nodejs/llhttp




# Usage

The usage of HttpServer is similar to TcpServer overall.

1. Create an EventLoop as the mainLoop

2. Create an InetAddress and bind the port number

3. Create an HttpServer and initialize it with the above two variables, register your own request handler function

4. Call the HttpServer's start function

5. Call the HttpServer's loop function

At this point, the HttpServer is ready. When new requests arrive and are parsed, your registered request handler function will be called, and you can access the complete request and perform your own business processing.




# Quick Start

```c++
#include <signal.h>

#include <explore/HttpServer.h>

void requestAcceptor(const TcpConnectionPtr &conn, const HttpRequest req)
{
    // Build a simple HTTP response
    HttpResponse response;

    response.setBody("<h1>Hello from C++17 Network Library!</h1><p>You requested: " + req.url + "</p>");
    response.setVersion("HTTP/1.1");
    response.setStatusCode(HttpStatusCode::k200OK);
    response.setStatusMessage("Content-Type: text/html");

    Buffer buf;
    response.writeToBuffer(&buf);
    conn->send(&buf);
}

int main()
{
    ::signal(SIGPIPE, SIG_IGN);

    EventLoop loop;
    InetAddress addr(8080);

    Logger::instance().setLevel(LogLevel::NONE);

    HttpServer server(&loop, addr);

    server.setRequestAcceptor(requestAcceptor);
  
    server.setThreadNum(7);

    server.start();
    loop.loop();

    return 0;
}
```




# Testing

Test environment: i7-12700H (10 cores 20 threads), Ubuntu 22.04, JMeter 5.6.3

Server settings: Logging output disabled, 7 subThreads enabled

The test code is the same as in Quick Start

JMeter test parameters: 20 threads, infinite loops, 60s test duration

Test results are as follows:

Aggregate Report:

| Label    | # Samples | Average | Median | 90th Percentile | 95th Percentile | 99th Percentile | Min | Max | Error % | Throughput  | Received KB/sec | Sent KB/sec |
| -------- | --------- | ------- | ------ | --------------- | --------------- | --------------- | --- | --- | ------- | ----------- | --------------- | ----------- |
| HTTP请求 | 5694963   | 0       | 0      | 1               | 1               | 1               | 0   | 75  | 0.00%   | 95123.73683 | 14305.72        | 11240.21    |
| Total    | 5694963   | 0       | 0      | 1               | 1               | 1               | 0   | 75  | 0.00%   | 95123.73683 | 14305.72        | 11240.21    |

Summary Report:

| Label    | # Samples | Average | Min | Max | Std. Deviation | Error % | Throughput  | Received KB/sec | Sent KB/sec | Avg. Bytes |
| -------- | --------- | ------- | --- | --- | -------------- | ------- | ----------- | --------------- | ----------- | ---------- |
| HTTP请求 | 5694963   | 0       | 0   | 75  | 0.46           | 0.00%   | 95123.73683 | 14305.72        | 11240.21    | 154        |
| Total    | 5694963   | 0       | 0   | 75  | 0.46           | 0.00%   | 95123.73683 | 14305.72        | 11240.21    | 154        |

Response Time Graph:

![响应时间图](../picture/http_响应时间图.png)



# HttpResponse Member Functions

1. void setStatusCode(HttpStatusCode statusCode) { statusCode_ = statusCode; }

   Set response status code

2. void setStatusMessage(const std::string &message) { statusMessage_ = message; }

   Set response status message

3. void setVersion(const std::string &version) { version_ = version; }

   Set HTTP version

4. void setHeaders(const std::unordered_map<std::string, std::string> &headers) { headers_ = headers; }

   Set response headers

5. void addHeaders(const std::string &key, const std::string &value) { headers_.insert_or_assign(key, value); }

   Add response header

6. void setBody(std::string body) { body_ = std::move(body); }

   Set response body

7. void setCloseConnection(bool close) { closeConnection_ = close; }

   Set close connection flag

8. void writeToBuffer(Buffer *output);

   Assemble response message to output