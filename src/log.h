#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <string>
#include <memory>
#include <sstream>

#define LOG(...) { std::stringstream __stream; __stream << __VA_ARGS__; log(__stream.str()); }
#define LOGW(...) { std::stringstream __stream; __stream << __VA_ARGS__; logw(__stream.str()); }
#define LOGE(...) { std::stringstream __stream; __stream << __VA_ARGS__; loge(__stream.str()); }
#define LOGC(...) { std::stringstream __stream; __stream << __VA_ARGS__; logc(__stream.str()); }
#define LOGD(...) { std::stringstream __stream; __stream << __VA_ARGS__; logd(__stream.str()); }

class Log;
using LoggerPtr = std::shared_ptr<Log>;

// Logging class
class Log
{
public:
	enum class Level
	{
		Critical,
		Error,
		Warning,
		Info,
		Debug
	};

private:
	static LoggerPtr _instance;
	Level _verb;
	std::ofstream _stream;

public:
	Log();
	~Log();

public:
	void setVerb(Level level) { _verb = level; }
	void write(const std::string &text, Log::Level level = Log::Level::Info);

	static LoggerPtr create()
	{
		if (_instance == nullptr)
			_instance = std::make_shared<Log>();
		return _instance;
	}

	static void close()
	{
		_instance.reset();
		_instance = nullptr;
	}

	static void put(const std::string &text, Log::Level level)
	{
		_instance->write(text, level);
	}

	static void setLevel(Level level)
	{
		_instance->setVerb(level);
	}

private:
	std::string createLine(const std::string &text, Log::Level level);
};

inline void log(const std::string &text, Log::Level level = Log::Level::Info) { Log::put(text, level); }
inline void logw(const std::string &text) { log(text, Log::Level::Warning); }
inline void loge(const std::string &text) { log(text, Log::Level::Error); }
inline void logc(const std::string &text) { log(text, Log::Level::Critical); }
inline void logd(const std::string &text) { log(text, Log::Level::Debug); }

#endif // LOG_H
