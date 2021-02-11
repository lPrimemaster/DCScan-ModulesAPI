#include <memory>
#include "../include/DCS_ModuleCore.h"

static void* aligned_malloc(size_t size, size_t alignment)
{
	void* p1;
	void** p2;
	size_t offset = alignment - 1 + sizeof(void*);
	if ((p1 = (void*)malloc(size + offset)) == NULL) return NULL;
	p2 = (void**)(((size_t)(p1)+offset) & ~(alignment - 1));
	p2[-1] = p1;
	return p2;
}

static void aligned_free(void* p)
{
	free(((void**)p)[-1]);
}

DCS::Memory::LinearAllocator::LinearAllocator(u64 size, u64 align)
{
	//buffer.data = buffer.data_start = (char*)aligned_malloc(size, align);
	buffer.data = buffer.data_start = (char*)malloc(size);
	if (buffer.data_start == nullptr)
	{
		Utils::Logger::Error("Failed to create linear allocator.");
	}
	else
	{
		Utils::Logger::Debug("Successfully created linear allocator. (size=%llu,align=%llu)", size, align);
	}

	buffer.alignment = align;
	buffer.size = size;
	buffer.usedSize = 0;
}

void DCS::Memory::LinearAllocator::release()
{
	//aligned_free(buffer.data_start);
	buffer.usedSize = 0;
	free(buffer.data_start);
}

void DCS::Memory::LinearAllocator::reset()
{
	buffer.usedSize = 0;
	buffer.data = buffer.data_start;
}
