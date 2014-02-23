#include "Common.h"
#include "Logger.h"

#define SELF_INDEX 0
#define ENEMY_INDEX 1

// constructor
Logger::Logger() 
{
	logFile = "C:\\Users\\Dave\\Desktop\\UalbertaBot_log.txt";
	logStream.open(logFile.c_str(), std::ofstream::app);
}

// get an instance of this
Logger & Logger::Instance() 
{
	static Logger instance;
	return instance;
}

void Logger::setLogFile(const std::string & filename)
{
	logFile = filename;
}

void Logger::log(const std::string & msg)
{
	logStream << msg;
}
