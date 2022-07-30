#pragma once

#include <cstdio>
#include <cstdarg>

#include <mutex>

#define LOG_PRINT(fmt, ...) g_log.println("%s(): " fmt, __FUNCTION__ , __VA_ARGS__)

struct Logger {
	Logger();

	bool initialize(const char* logFilePath);
	void println(const char* fmt, ...);

	std::mutex m_mutex;
	FILE* m_file;
};


extern Logger g_log;
