#include "../include/DCS_ModuleUtils.h"
#include "../include/internal.h"
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
DCS::Utils::Logger::Options DCS::Utils::Logger::options = DCS::Utils::Logger::Options::ENABLE_COLOR;
DCS::Utils::Logger::WriteNotifyCallback DCS::Utils::Logger::writenotify = nullptr;
void* DCS::Utils::Logger::obj = nullptr;

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

	// Only display message on cout/cerr if verbosity is less or equal than set value.
	if (v <= verbosity_level)
	{
		// Buffer is not newline terminated
		if(options == Options::ENABLE_COLOR)
			std::cout << buffer[1] << std::endl;
		else
			std::cout << buffer[0] << std::endl;

		// Send a message to the client with the latest log message
		if (writenotify != nullptr)
		{
			if (obj != nullptr)
			{
				writenotify(buffer[0].c_str(), obj);
			}
			else
			{
				writenotify(buffer[0].c_str(), nullptr);
			}
		}
	}

	// If there is a file to write the data to...
	if (handle)
	{
		fwrite((buffer[0] + '\n').c_str(), sizeof(char), buffer[0].size()+1, handle);
	}
}

void DCS::Utils::Logger::Settings(Options opt)
{
	options = opt;
}

void DCS::Utils::Logger::Init(Verbosity level, DCS::Utils::String file)
{
	verbosity_level = level;
	DWORD dwMode;
	HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(outHandle, &dwMode);
	dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(outHandle, dwMode);

	int x, y;
	GetConsoleSize(&x, &y);
	SetConsoleMargins(1, y - 1);

	WriteConsoleLine(y, "> ");

	Logger::handle = fopen(file.c_str(), "w");
}

void DCS::Utils::Logger::Destroy()
{
	fclose(Logger::handle);
}

void DCS::Utils::Logger::SetLogWriteCallback(DCS::Utils::Logger::WriteNotifyCallback wnc, void* obj)
{
	writenotify = wnc;
	Logger::obj = obj;
}

void DCS::Utils::Logger::Debug(const char* file, const char* msg, ...)
{
	size_t size = strlen(msg);
	if (size > 2048) 
		LOG_ERROR("Logger output is limited to 2048 bytes per call. Output might be different from expected.");

	char buffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	std::string s[] = {
		"[" + timestamp() + "][" + file + "][DEBUG/INFO]: " + buffer, // Color Stripped buffer
		LOGGER_GREY + s[0] + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::DEBUG);
}

void DCS::Utils::Logger::Message(const char* file, const char* msg, ...)
{
	size_t size = strlen(msg);
	if (size > 2048)
		LOG_ERROR("Logger output is limited to 2048 bytes per call. Output might be different from expected.");

	char buffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	std::string s[] = {
		"[" + timestamp() + "][" + file + "][MESSAGE]: " + buffer, // Color Stripped buffer
		LOGGER_BLUE + s[0] + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::MESSAGE);
}

void DCS::Utils::Logger::Warning(const char* file, const char* msg, ...)
{
	size_t size = strlen(msg);
	if (size > 2048)
		LOG_ERROR("Logger output is limited to 2048 bytes per call. Output might be different from expected.");

	char buffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	std::string s[] = {
		"[" + timestamp() + "][" + file + "][WARN]: " + buffer, // Color Stripped buffer
		LOGGER_ORANGE + s[0] + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::WARNING);
}

void DCS::Utils::Logger::Error(const char* file, const char* msg, ...)
{
	size_t size = strlen(msg);
	if (size > 2048)
		LOG_ERROR("Logger output is limited to 2048 bytes per call. Output might be different from expected.");

	char buffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	std::string s[] = {
		"[" + timestamp() + "][" + file + "][ERROR]: " + buffer, // Color Stripped buffer
		LOGGER_BOLD LOGGER_RED + s[0] + LOGGER_DEFAULT // Color Normal buffer
	};
#undef ERROR
	WriteData(s, Verbosity::ERROR);
#define ERROR 0
}

void DCS::Utils::Logger::Critical(const char* file, const char* msg, ...)
{
	size_t size = strlen(msg);
	if (size > 2048)
		LOG_ERROR("Logger output is limited to 2048 bytes per call. Output might be different from expected.");

	char buffer[2048];
	va_list args;
	va_start(args, msg);
	vsprintf(buffer, msg, args);
	va_end(args);

	std::string s[] = {
		"[" + timestamp() + "][" + file + "][CRITICAL]: " + buffer, // Color Stripped buffer
		LOGGER_RED + s[0] + LOGGER_DEFAULT // Color Normal buffer
	};
	WriteData(s, Verbosity::CRITICAL);
}
