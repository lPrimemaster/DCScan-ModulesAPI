#include "../include/DCS_ModuleUtils.h"

DCS::Utils::String::String(const char* text)
{
	buffer_size = strlen(text) + 1;

	buffer = (char*)malloc(sizeof(char) * buffer_size);

	if (buffer == nullptr)
	{
		LOG_ERROR("Failed to allocate string memory.");
	}
	else
	{
		strcpy(buffer, text);
	}
}

DCS::Utils::String::String(const String& s)
{
	if (buffer_size > 0) free(buffer);

	buffer_size = strlen(s.buffer) + 1;

	buffer = (char*)malloc(sizeof(char) * buffer_size);

	if (buffer == nullptr)
	{
		LOG_ERROR("Failed to allocate string memory.");
	}
	else
	{
		strcpy(buffer, s.c_str());
	}
}

DCS::Utils::String& DCS::Utils::String::operator=(const DCS::Utils::String& s) noexcept
{
	if (buffer_size > 0) free(buffer);

	buffer_size = strlen(s.buffer) + 1;

	buffer = (char*)malloc(sizeof(char) * buffer_size);

	if (buffer == nullptr)
	{
		LOG_ERROR("Failed to allocate string memory.");
	}
	else
	{
		strcpy(buffer, s.c_str());
	}

	return *this;
}

DCS::Utils::String::~String()
{
	if (buffer != nullptr)
	{
		free(buffer);
	}
}