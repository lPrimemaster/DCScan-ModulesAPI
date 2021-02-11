#include "../include/DCS_ModuleUtils.h"

DCS::Utils::String::String(const char* text)
{
	buffer_size = strlen(text);

	buffer = (char*)malloc(sizeof(char) * buffer_size);

	if (buffer == nullptr)
	{
		std::cerr << "Failed to allocate string memory." << std::endl;
	}
	else
	{
		strcpy(buffer, text);
	}
}

DCS::Utils::String::String(const String& s)
{
	buffer_size = strlen(s.buffer);

	if (buffer_size > 0) free(buffer);

	buffer = (char*)malloc(sizeof(char) * buffer_size);

	if (buffer == nullptr)
	{
		std::cerr << "Failed to allocate string memory." << std::endl;
	}
	else
	{
		strcpy(buffer, s.c_str());
	}
}

DCS::Utils::String::~String()
{
	if (buffer != nullptr)
	{
		free(buffer);
	}
}