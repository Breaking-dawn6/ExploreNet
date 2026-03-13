#pragma once

// 该类的派生类可以正常拷贝与析构，但是不能进行拷贝构造与赋值

class nocopyable
{
public:
    nocopyable(const nocopyable &) = delete;
    nocopyable &operator=(const nocopyable &) = delete;

protected:
    nocopyable() = default;
    ~nocopyable() = default;
};