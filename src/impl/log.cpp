#include "log.hpp"
#include <cstring>
#include <cerrno>
#include <ctime>
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <unordered_map>
// 类中的私有静态成员变量必须在cpp文件中先声明一遍，然后才能在静态成员函数中访问
// 在 C++ 中，静态成员变量属于类本身而不是任何具体的对象。由于它们在类中只是声明而没有定义，因此需要在类外面对它们进行定义，这就相当于为静态成员变量分配存储空间并指定其初始值。
const char *Logger::levelMap[] = {"DEBUG", "NORMAL", "WARNING", "ERROR", "FATAL"};
const char *Logger::pname_ = nullptr;

Logger::Logger() {}

Logger::~Logger() {}

void Logger::setLoggerPname(const char *pname)
{
    Logger::pname_ = pname;
}

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
        exit(-1);
    }

    // 定义一个映射，将字符串日志级别映射到 LogLevel 枚举
    std::unordered_map<std::string, LogLevel> logLevelMap = {
        {"DEBUG", LogLevel::DEBUG},
        {"NORMAL", LogLevel::NORMAL},
        {"WARNING", LogLevel::WARNING},
        {"ERROR", LogLevel::ERROR},
        {"FATAL", LogLevel::FATAL}};

    std::string line;
    while (std::getline(configFile, line))
    {
        // 移除行首尾的空格（可选）
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // 查找映射中的日志级别，判断是否匹配
        if (logLevelMap.find(line) != logLevelMap.end() && logLevelMap[line] == level)
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

    snprintf(fixBuffer, sizeof(fixBuffer), "<%s>==[%s:%s:%d][%s]",
             levelMap[level], pname_, file, line, timestr);

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
    std::cout << fixBuffer << ":" << defBuffer << std::endl;
}
Logger &logger = Logger::getInstance(); // 初始化全局 Logger 实例引用，这样其他所有模块只要包含了log.hpp，就可以引用这个全局唯一的logger输出标准化的日志