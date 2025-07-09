#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <list>
#include <stdio.h>
#include <vector>
#include <time.h>

using namespace std;

typedef enum enLogLevel
{
    LOG_DEBUG = 1,
    LOG_INFO,
    LOG_ERROR,
    LOG_WARN,
    LOG_FATAL
} LogLevel;

const char LogTip[][8] = {"", "Debug", "Info", "Error", "Warn", "Fatal"};

class UtilsClass
{
public:
    UtilsClass();
    static std::list<string> split(char *str, const char *delim);
    static void getLogFileName(string &logFileName);
    static void writeLog(char *fileName, int logLevel, const char *pLogFormat);
    static void writeMsg(char *fileName, const char *pMsg);
    static std::string getDate();
    static std::string getDateTimeString();
    static std::string readFile(char *fileName);
};
