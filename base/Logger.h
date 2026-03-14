#pragma once

#include "Timestamp.h"

#include <string>
#include <cstdarg>
#include <mutex>
#include <fstream>
#include <iostream>
#include <atomic>

enum LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NONE
};

class Logger
{
public:
    static Logger &instance();
    void setLevel(LogLevel logLevel);
    LogLevel getLevel();
    void log(LogLevel level, const char *format, ...);
    void enableFileLog(const std::string &filename);
    void setTerminal(bool flag);
    void setFastRefresh(bool flag);

private:
    Logger();
    ~Logger();
    LogLevel logLevel_ = LogLevel::INFO;
    std::mutex mutex_;
    std::ofstream logFile_;
    std::atomic_bool enableTerminal_ = true;
    std::atomic_bool fastRefresh_ = false;
};

#define LOG_DEBUG(format, ...)                                              \
    do                                                                      \
    {                                                                       \
        if (Logger::instance().getLevel() <= LogLevel::DEBUG)               \
        {                                                                   \
            Logger::instance().log(LogLevel::DEBUG, format, ##__VA_ARGS__); \
        }                                                                   \
    } while (0);

#define LOG_INFO(format, ...)                                              \
    do                                                                     \
    {                                                                      \
        if (Logger::instance().getLevel() <= LogLevel::INFO)               \
        {                                                                  \
            Logger::instance().log(LogLevel::INFO, format, ##__VA_ARGS__); \
        }                                                                  \
    } while (0);

#define LOG_WARN(format, ...)                                              \
    do                                                                     \
    {                                                                      \
        if (Logger::instance().getLevel() <= LogLevel::WARN)               \
        {                                                                  \
            Logger::instance().log(LogLevel::WARN, format, ##__VA_ARGS__); \
        }                                                                  \
    } while (0);

#define LOG_ERROR(format, ...)                                              \
    do                                                                      \
    {                                                                       \
        if (Logger::instance().getLevel() <= LogLevel::ERROR)               \
        {                                                                   \
            Logger::instance().log(LogLevel::ERROR, format, ##__VA_ARGS__); \
        }                                                                   \
    } while (0);

#define LOG_FATAL(format, ...)                                              \
    do                                                                      \
    {                                                                       \
        if (Logger::instance().getLevel() <= LogLevel::FATAL)               \
        {                                                                   \
            Logger::instance().log(LogLevel::FATAL, format, ##__VA_ARGS__); \
        }                                                                   \
    } while (0);
