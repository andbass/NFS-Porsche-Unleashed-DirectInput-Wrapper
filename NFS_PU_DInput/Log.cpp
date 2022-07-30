
#include "Log.h"

#include <Windows.h>
#include <ctime>

Logger::Logger() : m_mutex(), m_file(nullptr) {}

bool Logger::initialize(const char* logFilePath) {
	std::lock_guard<std::mutex> guard(m_mutex);

	m_file = fopen(logFilePath, "w");
	if (m_file == nullptr) {
		fprintf(stderr, "Cannot open log file for writing with path '%s'\n", logFilePath);

		char debugStr[1024];
		sprintf(debugStr, "Cannot open log file for writing with path '%s'", logFilePath);
		MessageBoxA(nullptr, debugStr, "", MB_OK);

		return false;
	}

	return true;
}

void Logger::println(const char* fmt, ...) {
	std::lock_guard<std::mutex> guard(m_mutex);

	if (m_file == nullptr) {
		return;
	}

	time_t curTime = time(nullptr);

	char timeString[26];
	ctime_s(timeString, sizeof(timeString), &curTime);
	timeString[24] = '\0'; // hack to remove newline

	fprintf(m_file, "[%s] ", timeString);

	va_list args;

	va_start(args, fmt);
	vfprintf(m_file, fmt, args);
	va_end(args);

	fprintf(m_file, "\n");
	fflush(m_file);
}

// The default global logger, init'd by DllMain()
Logger g_log;