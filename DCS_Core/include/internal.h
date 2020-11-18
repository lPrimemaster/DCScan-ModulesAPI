#pragma once

namespace DCS
{
	namespace Memory
	{
		struct LinearDataPointer
		{
			char* data_start = nullptr;
			char* data = nullptr;
			u64 size;
			u64 alignment;
		};
	}
}