#pragma once

#include <iostream>
#include <string>

class Timestamp
{
private:
    int64_t microSecondsSinceEpoch_;

public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);

    explicit Timestamp(double seconds);

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    int64_t MicroSecondsSinceEpoch() { return microSecondsSinceEpoch_; }

    static Timestamp invalid() { return Timestamp(); }

    static Timestamp now();

    // 返回系统启动到现在的时间戳
    static Timestamp systemRunTime();

    std::string toString() const;

    std::string toFileNameString() const;

    Timestamp operator+(int64_t time) const;

    Timestamp operator+(double seconds) const;

    Timestamp operator+(const Timestamp &time) const;

    bool operator<(const Timestamp &time) const;

    bool operator==(const Timestamp &time) const;
};