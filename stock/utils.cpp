
#include "utils.h"
//extern "C" void __gcov_flush();

std::list<string> UtilsClass::split(char *str, const char *delim)
{
    std::list<string> elems;
    char *s = strtok(str, delim);
    while (s != NULL)
    {
        elems.push_back(s);
        s = strtok(NULL, delim);
    }
    return elems;
}

void UtilsClass::getLogFileName(string &logFileName)
{

    time_t t = time(0);
    char tmp[32] = {0};
    strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&t));
    strcat(tmp, ".log");
    char path[50] = "LogFile/";
    strcat(path, tmp);
    logFileName = path;
}

void UtilsClass::writeLog(char *fileName, int logLevel, const char *pLogFormat)
{

    string logFileName;
    if (strlen(fileName) < 3)
    {
        getLogFileName(logFileName);
    }
    else
    {
        logFileName = fileName;
    }
    std::ofstream OsWrite(logFileName, std::ofstream::app);
    char logTxt[2048];
    memset(logTxt, 0, sizeof(logTxt));

    time_t t = time(0);
    char date[32] = {0};
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&t));

    sprintf(logTxt, "%s : [%s], %s\n", date, LogTip[logLevel], pLogFormat);
    OsWrite << logTxt;
    // OsWrite << std::endl;
    OsWrite.close();
    //__gcov_flush();
}

void UtilsClass::writeMsg(char *fileName, const char *pMsg)
{

    string logFileName;
    if (strlen(fileName) < 3)
    {
        getLogFileName(logFileName);
    }
    else
    {
        logFileName = fileName;
    }
    std::ofstream OsWrite(logFileName, std::ofstream::app);
    OsWrite << pMsg;
    OsWrite.close();
    //__gcov_flush();
}

std::string UtilsClass::getDate()
{
    time_t t = time(0);
    char date[32] = {0};
    strftime(date, sizeof(date), "%Y-%m-%d", localtime(&t));
    return (std::string)date;
}

std::string UtilsClass::getDateTimeString()
{
    time_t t = time(0);
    char date[32] = {0};
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return (std::string)date;
}

std::string UtilsClass::readFile(char *fileName)
{
    std::ifstream ifs(fileName);
    std::stringstream sbuffer;
    sbuffer << ifs.rdbuf();
    ifs.close();
    return sbuffer.str();
}