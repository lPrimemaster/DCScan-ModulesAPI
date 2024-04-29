#include "../include/DCS_ModuleUtils.h"
#include <cstring>

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

DCS::Utils::String::String(const char* ptr, const size_t size)
{
	buffer_size = size;

	buffer = (char*)malloc(sizeof(char) * (buffer_size + 1));

	if (buffer == nullptr)
	{
		LOG_ERROR("Failed to allocate string memory.");
	}
	else
	{
		memcpy(buffer, ptr, buffer_size);
		buffer[buffer_size] = '\0';
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

DCS::Utils::Vector<DCS::Utils::String> DCS::Utils::String::split(const char separator)
{
	Vector<String> output;

	int offset = 0;
	for(int i = 0; i < buffer_size; i++)
	{
		if(buffer[i] == separator)
		{
			output.emplace(&buffer[offset], (size_t)(i - offset));
			offset = i + 1;
		}
	}
	output.emplace(&buffer[offset], (size_t)(buffer_size - offset));

	return output;
}
