#pragma once

#include <vector>
#include <string>
#include <algorithm>

// 网络库底层的缓冲区类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writeIndex_(kCheapPrepend)
    {
    }

    size_t readableBytes() const { return writeIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writeIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 返回缓冲区中可读数据的起始地址
    const char *peek() const { return begin() + readerIndex_; }

    // onMessage string <- Buffer
    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len; // 应用只读取了可读缓冲区数据的一部分,即len
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = writeIndex_ = kCheapPrepend;
    }

    // 把onMessage函数上报的buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes()); // 应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 上面一句把缓冲区中可读的数据已经读取出来，这里需要对缓冲区进行复位操作
        return result;
    }

    std::string retrieveAllAsStringTrimmed()
    {
        return retrieveAsStringTrimmed(readableBytes());
    }

    std::string retrieveAsStringTrimmed(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        {
            result.pop_back();
        }
        return result;
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    char *beginWrite()
    {
        return begin() + writeIndex_;
    }

    const char *beginWrite() const
    {
        return begin() + writeIndex_;
    }

    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);

    // 从fd上发送数据
    ssize_t writeFd(int fd, int *saveErrno);

    // 把[data,data+len]内存上的数据，添加到writeable缓冲区当中
    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writeIndex_ += len;
    }

    // 从fd上读取数据
    ssize_t wtiteFd(int fd, int *savedErrno);

private:
    char *begin() { return buffer_.data(); }
    const char *begin() const { return buffer_.data(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() - kCheapPrepend < len)
        {
            buffer_.resize(writeIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writeIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writeIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writeIndex_;
};