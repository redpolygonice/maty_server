#include "log.h"

#include <iostream>
#include <ctime>
#include <sys/time.h>

LoggerPtr Log::_instance = nullptr;

Log::Log()
	: _verb(Level::Debug)
{
	std::string logName = "log-" + currentTime() + ".txt";

	_stream.open(logName, std::ios_base::out | std::ios_base::trunc);
	if (!_stream.is_open())
		return;

	_stream << "[" << currentTimeMs() << "] " << "Start"  << std::endl;
}

Log::~Log()
{
	if (_stream.is_open())
	{
		_stream << "[" << currentTimeMs() << "] " << "Quit"  << std::endl;
		_stream.close();
	}
}

void Log::write(const std::string &text, Log::Level level)
{
	if (level > _verb)
		return;

	std::string line = createLine(text, level);
	std::cout << line << std::endl;
	_stream << line << std::endl;
	_stream.flush();
}

std::string Log::createLine(const std::string &text, Log::Level level)
{
	std::string prefix = "";
	switch (level)
	{
	case Log::Level::Info:
		prefix = "Info!    ";
		break;
	case Log::Level::Warning:
		prefix = "Warning! ";
		break;
	case Log::Level::Error:
		prefix = "Error!   ";
		break;
	case Log::Level::Critical:
		prefix = "Critical!!! ";
		break;
	case Log::Level::Debug:
		prefix = "Debug!   ";
		break;
	}

	std::string line = "[" + currentTimeMs() + "] " + prefix + text;
	return line;
}

std::string Log::currentTime()
{
	std::time_t time = std::time(NULL);
	char timeStr[50];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
	return timeStr;
}

std::string Log::currentTimeMs()
{
	char timeStr[50];
	struct timeval tv;
	gettimeofday(&tv, NULL);
	std::time_t now = tv.tv_sec;
	struct tm *tm = std::localtime(&now);

	if (tm == nullptr)
		return currentTime();

	sprintf(timeStr, "%04d-%02d-%02d_%02d-%02d-%02d.%03d",
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			static_cast<int>(tv.tv_usec / 1000));

	return timeStr;
}
