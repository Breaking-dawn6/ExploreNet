#include "Timestamp.h"

#include <time.h>
#include <math.h>
#include <chrono>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

Timestamp::Timestamp(double seconds) : microSecondsSinceEpoch_(round(seconds * kMicroSecondsPerSecond)) {}

Timestamp Timestamp::now()
{
    return Timestamp(time(NULL) * kMicroSecondsPerSecond);
}

Timestamp Timestamp::systemRunTime()
{
    return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}

std::string Timestamp::toFileNameString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&microSecondsSinceEpoch_);
    snprintf(buf, 128, "%4d%02d%02d_%02d%02d%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}

Timestamp Timestamp::operator+(int64_t time) const
{
    return Timestamp(this->microSecondsSinceEpoch_ + time);
}

Timestamp Timestamp::operator+(double seconds) const
{
    int64_t delta = static_cast<int64_t>(round(seconds * kMicroSecondsPerSecond));
    return Timestamp(microSecondsSinceEpoch_ + delta);
}

Timestamp Timestamp::operator+(const Timestamp &time) const
{
    return Timestamp(this->microSecondsSinceEpoch_ + time.microSecondsSinceEpoch_);
}

bool Timestamp::operator<(const Timestamp &time) const
{
    return this->microSecondsSinceEpoch_ < time.microSecondsSinceEpoch_;
}

bool Timestamp::operator==(const Timestamp &time) const
{
    return this->microSecondsSinceEpoch_ == time.microSecondsSinceEpoch_;
}

// #include <iostream>
// int main(void)
// {
//     std::cout << Timestamp::now().toString() << std::endl;

//     return 0;
// }