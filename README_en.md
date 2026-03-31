<center><a href="README.md">简体中文</a> | <a href="README_en.md">English</a></center>

# Overview

This project is a reimplementation of part of the Muduo library's core network components with C++17, removing Boost dependencies.

The project framework was initially inspired by Prof. Shi Lei's course on writing a C++ Muduo network library from scratch. 

Notes taken during the course can be found in [MyMuduo学习笔记.md](./assets/docs/MyMuduo学习笔记.md).

After completing the course, I upgraded the C++ standard from C++11 to C++17, rewrote a logging class, converted all `std::bind` to lambda expressions, fixed bugs in the course code under high concurrency scenarios, and made some optimizations.

This project is currently for learning purposes only. I plan to develop projects based on this library and will make corrections or extensions to the library as projects progress.


# Build

The project currently has a build script `autobuild.sh` that accepts at most one argument.

1. `autobuild.sh ON` or `autobuild.sh`

   This build method will build the entire project, including the HTTP server. Before building, ensure that llhttp headers are in `/usr/local/include` and llhttp dynamic libraries are in `/usr/local/lib`.

2. `autobuild.sh OFF`

   This build method will only build the core network modules of the project, excluding HTTP and other extensions.

In both build modes, the script will copy the compiled dynamic libraries to `/usr/lib` and copy all project header files to `/usr/include/explore`.


# Usage

The usage of this project is similar to Muduo. However, according to modern CPU performance, I have adjusted the default number of worker threads to 1. This means that without additional configuration, the project will now start with 1 mainThread and 1 subThread by default.

1. Create an EventLoop as the mainLoop
2. Create an InetAddress and bind the port number
3. Create a Server and initialize it with the above two variables, while registering your own callback functions
4. Call the TcpServer's start function
5. Call the mainLoop's loop function

At this point, a simple server has been started.


# Quick Start

```cpp
// Simple Echo Server
#include <explore/TcpServer.h>
#include <explore/Logger.h>

#include <string>
#include <functional>
#include <signal.h>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
               const InetAddress &addr,
               const std::string name)
        : server_(loop, addr, name),
          loop_(loop),
          exitStr_("quit")
    {
        // Register callback functions
        server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                      { onConnection(conn); });
        server_.setMessageCallback([this](const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
                                   { onMessage(conn, buf, time); });

        // Set appropriate number of threads (subThread)
        server_.setThreadNum(7);
    }

    void start()
    {
        server_.start();
    }

private:
    // Callback for connection establishment or termination
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            LOG_INFO("conn UP : %s\n", conn->peerAddress().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("conn Down : %s\n", conn->peerAddress().toIpPort().c_str());
        }
    }

    // Readable/writable event callback
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buf,
                   Timestamp time)
    {
        conn->send(buf->peek(), buf->readableBytes());
        buf->retrieveAll();
    }

    EventLoop *loop_; // mainLoop
    TcpServer server_;
    std::string exitStr_;
};

int main(void)
{
    ::signal(SIGPIPE, SIG_IGN);

    Logger::instance().setLevel(LogLevel::NONE);
    // Logger::instance().setTerminal(false);
    // Logger::instance().enableFileLog("echoServer");
    // Logger::instance().setFastRefresh(true);

    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01"); // Acceptor non-blocking listenfd create bind
    server.start();                                  // listen loopthread listenfd -> acceptChannel -> mainLoop -> newConnection
    loop.loop();                                     // Start mainLoop's underlying Poller

    return 0;
}
```

Compile command (GCC)

```bash
g++ -o echoServer echoServer.cpp -lexplore -lpthread -g -std=c++17
```

Run

```bash
./echoServer
```

Connect

```bash
telnet 127.0.0.1 8000
```


# Testing

I created a simple EchoServer for stress testing. This server echoes back received messages as-is.

Test environment: i7-12700H (10 cores 20 threads), Ubuntu 22.04, JMeter 5.6.3

Server settings: Logging output disabled, 7 subThreads enabled

Two types of tests were conducted:

## 1. Long Connection Throughput Test

JMeter test parameters: 12 threads, infinite loops, 60s test duration, connection reuse enabled, close connection disabled

Test results are as follows:

Aggregate Report:

| Label         | # Samples | Average | Median | 90th Percentile | 95th Percentile | 99th Percentile | Min | Max | Error % | Throughput  | Received KB/sec | Sent KB/sec |
| ------------- | --------- | ------- | ------ | --------------- | --------------- | --------------- | --- | --- | ------- | ----------- | --------------- | ----------- |
| Test LongConn | 8909511   | 0       | 0      | 0               | 1               | 1               | 0   | 21  | 0.00%   | 148486.9004 | 4785.22         | 0           |
| Total         | 8909511   | 0       | 0      | 0               | 1               | 1               | 0   | 21  | 0.00%   | 148486.9004 | 4785.22         | 0           |

Summary Report:

| Label         | # Samples | Average | Min | Max | Std. Deviation | Error % | Throughput  | Received KB/sec | Sent KB/sec | Avg. Bytes |
| ------------- | --------- | ------- | --- | --- | -------------- | ------- | ----------- | --------------- | ----------- | ---------- |
| Test LongConn | 8909511   | 0       | 0   | 21  | 0.26           | 0.00%   | 148486.9004 | 4785.22         | 0           | 33         |
| Total         | 8909511   | 0       | 0   | 21  | 0.26           | 0.00%   | 148486.9004 | 4785.22         | 0           | 33         |

Response Time Graph:

![Response Time Graph_long12_60](assets/picture/响应时间图_long12_60.png)


## 2. Short Connection Stability Test

JMeter test parameters: 12 threads, infinite loops, 600s test duration, two samplers. The Echo client sends a message ("hello,here is JMeter") to the server and then disconnects. The Exit client disconnects immediately after connecting. Connection reuse was not enabled for either.

Test results are as follows:

Aggregate Report:

| Label     | # Samples | Average | Median | 90th Percentile | 95th Percentile | 99th Percentile | Min | Max | Error % | Throughput  | Received KB/sec | Sent KB/sec |
| --------- | --------- | ------- | ------ | --------------- | --------------- | --------------- | --- | --- | ------- | ----------- | --------------- | ----------- |
| Test exit | 4634674   | 0       | 0      | 1               | 5               | 6               | 0   | 26  | 0.00%   | 7724.43092  | 7.54            | 0           |
| Test echo | 4634669   | 0       | 0      | 1               | 5               | 6               | 0   | 22  | 0.00%   | 7724.43546  | 158.41          | 0           |
| Total     | 9269343   | 0       | 0      | 1               | 5               | 6               | 0   | 26  | 0.00%   | 15448.82776 | 165.95          | 0           |

Summary Report:

| Label     | # Samples | Average | Min | Max | Std. Deviation | Error % | Throughput  | Received KB/sec | Sent KB/sec | Avg. Bytes |
| --------- | --------- | ------- | --- | --- | -------------- | ------- | ----------- | --------------- | ----------- | ---------- |
| Test exit | 4634674   | 0       | 0   | 26  | 1.41           | 0.00%   | 7724.43092  | 7.54            | 0           | 1          |
| Test echo | 4634669   | 0       | 0   | 22  | 1.41           | 0.00%   | 7724.43546  | 158.41          | 0           | 21         |
| Total     | 9269343   | 0       | 0   | 26  | 1.41           | 0.00%   | 15448.82776 | 165.95          | 0           | 11         |

Response Time Graph:

![Response Time Graph_short12_600](assets/picture/响应时间图_short12_600.png)


# Protocol Support

## 1. HTTP Support

For details, see [HTTP_en.md](./assets/docs/HTTP_en.md)




# Future Plans

1. Implement a Timer class based on the Linux timer, and use it to implement asynchronous logging and a monitoring thread (Monitor)

2. Extend HTTP protocol support based on the llhttp library (preliminary completion)

3. Expand IPv6 support (TBD)
