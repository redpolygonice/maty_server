#include "common.h"

#include <sys/time.h>
#include <ctime>
#include <chrono>

#include <unistd.h>
#include <sys/types.h>
#include <fstream>

std::string currentTime()
{
	std::time_t time = std::time(NULL);
	char timeStr[50];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
	return timeStr;
}

std::string currentTimeMs()
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

int64_t timestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t timestamp_micro()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool isQtCreatorParentProc()
{
	std::string ppid = std::to_string(getppid());
	std::string fileName = "/proc/" + ppid + "/comm";

	std::ifstream ifs(fileName, std::ios::in);
	if (!ifs.is_open())
		return false;

	std::string line;
	std::getline(ifs, line);

	if (line.find("qtcreator") != std::string::npos ||
		line.find("gdb") != std::string::npos)
		return true;

	return false;
}
