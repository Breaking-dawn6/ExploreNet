#include "Logger.h"

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel logLevel)
{
    logLevel_ = logLevel;
}

LogLevel Logger::getLevel()
{
    return logLevel_;
}

void Logger::log(LogLevel level, const char *format, ...)
{
    if (level < logLevel_)
    {
        return;
    }
    char buf[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    std::string levelStr;
    switch (level)
    {
    case LogLevel::DEBUG:
        levelStr = "[DEBUG]";
        break;
    case LogLevel::INFO:
        levelStr = "[INFO]";
        break;
    case LogLevel::WARN:
        levelStr = "[WARN]";
        break;
    case LogLevel::ERROR:
        levelStr = "[ERROR]";
        break;
    case LogLevel::FATAL:
        levelStr = "[FATAL]";
        break;
    }

    std::string logLine = levelStr + " " + Timestamp::now().toString() + " : " + buf + "\n";

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (enableTerminal_)
        {
            std::cout << logLine;
        }
        if (logFile_.is_open())
        {
            logFile_ << logLine;
            if (fastRefresh_)
            {
                logFile_ << std::flush;
            }
            else if (level >= LogLevel::ERROR)
            {
                logFile_ << std::flush;
            }
        }
    }

    if (level == LogLevel::FATAL)
    {
        exit(-1);
    }
}

void Logger::enableFileLog(const std::string &filename)
{
    if (filename.empty())
        return;

    std::string finalName = filename + "_" + Timestamp::now().toFileNameString() + ".log";

    {
        std::lock_guard<std::mutex> lock(mutex_);
        logFile_.open(finalName.c_str(), std::ios::out | std::ios::app);
    }

    if (logFile_.is_open())
    {
        log(INFO, "%s logFile has been opened", finalName.c_str());
    }
    else
    {
        setTerminal(true);
        log(ERROR, "logFile opening failed: %s", finalName.c_str());
    }
}

void Logger::setTerminal(bool flag)
{
    enableTerminal_ = flag;
}

void Logger::setFastRefresh(bool flag)
{
    fastRefresh_ = flag;
}

Logger::Logger() : enableTerminal_(true)
{
}

Logger::~Logger()
{
    if (logFile_.is_open())
    {
        logFile_.close();
    }
}
