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

DCS::Utils::String::~String()
{
	if (buffer != nullptr)
	{
		free(buffer);
	}
}