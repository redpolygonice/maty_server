#include "log.h"
#include "common.h"
#include "settings.h"

#include <iostream>
#include <ctime>
#include <sys/time.h>

LoggerPtr Log::_instance = nullptr;

Log::Log()
	: _verb(Level::Debug)
{
#ifdef WIN32
	std::string logName = GetSettings()->logPath().toStdString() +
						  "\\log-" + currentTime() + ".txt";
#else
	std::string logName = GetSettings()->logPath().toStdString() +
						  "/log-" + currentTime() + ".txt";
#endif

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
