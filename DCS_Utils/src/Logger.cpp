#include "../include/DCS_ModuleUtils.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <ctime>

#define LOGGER_DEFAULT "\x1b[0m"
#define LOGGER_RED     "\x1b[31m"
#define LOGGER_ORANGE  "\x1b[33m"
#define LOGGER_BLUE    "\x1b[34m"
#define LOGGER_BOLD    "\x1b[1m"
#define LOGGER_GREY    "\x1b[90m"

std::mutex DCS::Utils::Logger::_log_mtx;
FILE* DCS::Utils::Logger::handle = nullptr;
DCS::Utils::Logger::Verbosity DCS::Utils::Logger::verbosity_level = DCS::Utils::Logger::Verbosity::DEBUG;

std::string DCS::Utils::Logger::timestamp()
{
	// Get the time
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	std::time_t tt = std::chrono::system_clock::to_time_t(tp);
	std::tm* gmt = gmtime(&tt);
	std::chrono::duration<double> fractional_seconds =
		(tp - std::chrono::system_clock::from_time_t(tt)) + std::chrono::seconds(gmt->tm_sec);
	// Format the string
	std::string buffer("year/mo/dy hr:mn:sc.xxxxxx");
	sprintf(&buffer.front(), "%04d/%02d/%02d %02d:%02d:%09.6f", gmt->tm_year + 1900, gmt->tm_mon + 1,
		gmt->tm_mday, gmt->tm_hour, gmt->tm_min, fractional_seconds.count());
	return buffer;
}

void DCS::Utils::Logger::WriteData(std::string buffer[], Verbosity v)
{
	const std::lock_guard<std::mutex> lock(_log_mtx);

	// Only display message on cerr if verbosity is less or equal than set value.
	if (v <= verbosity_level)
	{
		// Buffer is already newline terminated
		std::cout << buffer[1] << std::endl;
	}

	// If there is a file to write the data to...
	if (handle)
	{
		fwrite(buffer[0].c_str(), sizeof(char), buffer[0].size(), handle);
	}
}

void DCS::Utils::Logger::Init(Verbosity level, FILE* handle)
{
	verbosity_level = level;
	DWORD dwMode;
	HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(outHandle, &dwMode);
	dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(outHandle, dwMode);

	Logger::handle = handle;

	// Disable this behaviour later on program end
}

void DCS::Utils::Logger::Debug(const char* msg)
{
	std::string s[] = {
		"[" + timestamp() + "][DEBUG/INFO]: " + msg, // Color Stripped buffer
		LOGGER_GREY "[" + timestamp() + "][DEBUG/INFO]: " + msg + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::DEBUG);
}

void DCS::Utils::Logger::Message(const char* msg)
{
	std::string s[] = {
		"[" + timestamp() + "][MESSAGE]: " + msg, // Color Stripped buffer
		LOGGER_BLUE "[" + timestamp() + "][MESSAGE]: " + msg + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::MESSAGE);
}

void DCS::Utils::Logger::Warning(const char* msg)
{
	std::string s[] = {
		"[" + timestamp() + "][WARN]: " + msg, // Color Stripped buffer
		LOGGER_ORANGE "[" + timestamp() + "][WARN]: " + msg + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::WARNING);
}

void DCS::Utils::Logger::Error(const char* msg)
{
	std::string s[] = {
		"[" + timestamp() + "][ERROR]: " + msg, // Color Stripped buffer
		LOGGER_RED "[" + timestamp() + "][ERROR]: " + msg + LOGGER_DEFAULT // Color Normal buffer
	};
#undef ERROR
	WriteData(s, Verbosity::ERROR);
#define ERROR 0
}

void DCS::Utils::Logger::Critical(const char* msg)
{
	std::string s[] = {
		"[" + timestamp() + "][CRITICAL]: " + msg, // Color Stripped buffer
		LOGGER_BOLD LOGGER_RED "[" + timestamp() + "][CRITICAL]: " + msg + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::CRITICAL);
}
