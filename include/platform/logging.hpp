#pragma once

#ifndef NDEBUG
#include <spdlog/spdlog.h>
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__);
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__);
#define LOG_INFO(...) spdlog::info(__VA_ARGS__);
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__);
#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__);

#else

#define LOG_ERROR(...)
#define LOG_WARN(...)
#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_TRACE(...)

#endif

enum LogLevel {
	ERROR,
	WARN,
	INFO,
	DEBUG,
	TRACE
};

void SetLogLevel(LogLevel ll);

void ToggleLogLevel();