#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;

	using namespace DCS::Memory;

	DCS::u64 size = 2 * sizeof(int);

	LinearAllocator la = LinearAllocator::New(size);

	int* a = la.allocate<int>(3);
	int* b = la.allocate<int>(4);

	DCS_ASSERT_EQ(*a, 3);
	DCS_ASSERT_EQ(*b, 4);

	la.reset();

	int* c = la.allocate<int>(5);
	int* d = la.allocate<int>(6);

	DCS_ASSERT_EQ(*c, 5);
	DCS_ASSERT_EQ(*d, 6);

	la.release();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
	//return 0;
}