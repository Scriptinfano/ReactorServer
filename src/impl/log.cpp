#include "log.hpp"
#include <cstring>
#include <cerrno>
#include <ctime>
#include <cstdarg>
#include <iostream>
#include <sstream>

const char *Logger::levelMap[] = {"DEBUG", "NORMAL", "WARNING", "ERROR", "FATAL"};

Logger::Logger() {}

Logger::~Logger() {}

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

std::string Logger::createErrorMessage(const std::string &msg)
{
    const char *error_str = strerror(errno);
    return msg + ": " + error_str;
}

bool Logger::shouldLogToFile(LogLevel level)
{
    std::ifstream configFile(CONFIGFILE);
    if (!configFile.is_open())
    {
        std::cerr << "Failed to open config file: " << CONFIGFILE << std::endl;
        return false;
    }

    int configLevel;
    std::string line;
    while (std::getline(configFile, line))
    {
        if (std::istringstream(line) >> configLevel && configLevel == static_cast<int>(level))
        {
            return true;
        }
    }
    return false;
}

void Logger::logMessage(LogLevel level, const char *file, int line, const char *format, ...)
{
    if (level < DEBUG || level > FATAL)
    {
        std::cerr << "Invalid log level: " << level << std::endl;
        return;
    }

    char fixBuffer[512];
    std::time_t currentTime = std::time(nullptr);
    char timestr[64];
    if (std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime)) == 0)
    {
        std::cerr << "Failed to format time." << std::endl;
        return;
    }

    snprintf(fixBuffer, sizeof(fixBuffer), "<%s>==[file->%s][line->%d][time->%s]",
             levelMap[level], file, line, timestr);

    char defBuffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(defBuffer, sizeof(defBuffer), format, args);
    va_end(args);

    if (shouldLogToFile(level))
    {
        std::ofstream logFile(LOGFILE, std::ios_base::app);
        if (!logFile.is_open())
        {
            std::cerr << "Failed to open log file: " << LOGFILE << std::endl;
            return;
        }
        logFile << fixBuffer << ":" << defBuffer << std::endl;
    }
    else
    {
        std::cout << fixBuffer << ":" << defBuffer << std::endl;
    }
}
Logger &logger = Logger::getInstance(); // 初始化全局 Logger 实例引用，这样其他所有模块只要包含了log.hpp，就可以引用这个全局唯一的logger输出标准化的日志