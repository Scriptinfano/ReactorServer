#pragma once
#include <string>
#include <fstream>

const std::string LOGFILE = "./log/log.txt";       // 日志输出文件路径
const std::string CONFIGFILE = "./etc/config.txt"; // 配置文件路径

enum LogLevel
{
    DEBUG,
    NORMAL,
    WARNING,
    ERROR,
    FATAL
};
/*
日志打印类
*/
class Logger
{
public:
    static Logger &getInstance();

    // 创建错误信息
    std::string createErrorMessage(const std::string &msg);

    // 输出日志消息
    void logMessage(LogLevel level, const char *file, int line, const char *format, ...);

private:
    Logger();
    ~Logger();
    bool shouldLogToFile(LogLevel level);

    static const char *levelMap[];
};

extern Logger &logger; // 声明一个全局 Logger 实例引用