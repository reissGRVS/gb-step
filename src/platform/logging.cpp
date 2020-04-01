#include "platform/logging.hpp"

#ifdef NDEBUG
#include <iostream>
#include <spdlog/spdlog.h>
LogLevel logLevel = LogLevel::INFO;
#endif

void SetLogLevel(LogLevel ll)
{
#ifdef NDEBUG
	switch (ll) {
	case ERROR:
		spdlog::set_level(spdlog::level::level_enum::err);
		break;
	case WARN:
		spdlog::set_level(spdlog::level::level_enum::warn);
		break;
	case INFO:
		spdlog::set_level(spdlog::level::level_enum::info);
		break;
	case DEBUG:
		spdlog::set_level(spdlog::level::level_enum::debug);
		break;
	case TRACE:
		spdlog::set_level(spdlog::level::level_enum::trace);
		break;
	}
#endif
}

void ToggleLogLevel()
{
#ifdef NDEBUG
	if (logLevel == TRACE) {
		std::cout << "Toggled to INFO log level" << std::endl;
		logLevel = INFO;
	} else if (logLevel == INFO) {
		std::cout << "Toggled to DEBUG log level" << std::endl;
		logLevel = DEBUG;
	} else {
		std::cout << "Toggled to TRACE log level" << std::endl;
		logLevel = TRACE;
	}

	SetLogLevel(logLevel);
#endif
}
