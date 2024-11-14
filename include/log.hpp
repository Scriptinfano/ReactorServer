#pragma once
#include <string>
#include <fstream>
/*
一个典型的日志文件的文件名如下：
logfile_test.2012060-144022.hostname.3605.log
文件名由以下几部分组成：
第1部分logfile_test是进程的名字。通常是main()函数参数中argv[0]的basename(3)，这样容易区分究竟是哪个服务程序的日志。必要时还可以把程序版本加进去。
第2部分是文件的创建时间（GMT时区）。这样很容易通过文件名来选择某一时间范围内的日志，例如用通配符*.20120603-14*表示2012年6月3日下午2点（GMT）左右的日志文件(s)。
第3部分是机器名称。这样即便把日志文件拷贝到别的机器上也能追溯其来源。
第4部分是进程id。如果一个程序一秒之内反复重启，那么每次都会生成不同的日志文件，参考§9.4。
第5部分是统一的后缀名.log。同样是为了便于周边配套脚本的编写。


*/
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
    static void setLoggerPname(const char *pname);
    // 创建错误信息
    std::string createErrorMessage(const std::string &msg);

    // 输出日志消息
    void logMessage(LogLevel level, const char *file, int line, const char *format, ...);

private:
    Logger();
    ~Logger();
    bool shouldLogToFile(LogLevel level);
    static const char *pname_;//整个程序的main函数的argv[]的第一个参数，也就是程序的名字是什么
    static const char *levelMap[];
};

extern Logger &logger; // 声明一个全局 Logger 实例引用