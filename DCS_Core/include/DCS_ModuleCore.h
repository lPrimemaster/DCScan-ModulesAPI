#pragma once
#include "../../config/exports.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"
#include "../../DCS_Core/include/internal.h"

namespace DCS
{
	namespace Memory
	{
		struct DCS_API LinearAllocator
		{
		public:
			static LinearAllocator New(u64 size, u64 align = 1);

			template<typename T, typename... Args>
			T* allocate(Args... args)
			{
				T* rb = new(buffer.data) T(args...);
				buffer.data += sizeof(T);
				return rb;
			}

			template<typename T>
			T* allocate()
			{
				T* rb = new(buffer.data) T();
				buffer.data += sizeof(T);
				return rb;
			}

			void release();
			void reset();

		private:
			LinearAllocator(u64 size, u64 align);

		private:
			LinearDataPointer buffer;
		};
	}
}