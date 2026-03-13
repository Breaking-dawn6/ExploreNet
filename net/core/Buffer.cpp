#include "Buffer.h"

#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

// 从fd上读取数据，poller工作在LT模式（数据未读完会一直上报）
// buffer缓冲区是有大小的！但是从fd上读数据的时候，却不知道tcp数据最终的大小

ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    char extraBuf[65536] = {0}; // 栈上的内存空间 64K
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    // 一次最多读64K
    const int iovcnt = (writable < sizeof(extraBuf) ? 2 : 1);
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (n <= writable) // 未用到extraBuffer
    {
        writeIndex_ += n;
    }
    else // 使用了extraBuffer
    {
        writeIndex_ = buffer_.size();
        append(extraBuf, n - writable); // 从writeIndex开始写n-writable大小的数据
    }

    return n;
}

// 从fd上发送数据
ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n <= 0)
    {
        *saveErrno = errno;
    }
    return n;
}