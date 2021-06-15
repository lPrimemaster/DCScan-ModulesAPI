#include "../include/DCS_ModuleCore.h"
#include <iostream>
#include <functional>
#include <algorithm>

typedef bool (*CompareFunc)(DCS::f64, DCS::f64);

struct Cmpdata
{
	size_t index;
	enum Edge{ RISING, FALLING } edge;
};

static size_t waitForLimit(DCS::f64* data, size_t start, size_t size, DCS::f64 barrier, CompareFunc cmp)
{
	size_t idx = start;
	while (true)
	{
		if (cmp(data[idx], barrier)) 
			return idx;
		else if (idx < size)
		{
			idx++;
			continue;
		}
		return size + 1;
	}
}

#define CMP_LARGER  [](DCS::f64 a, DCS::f64 b) -> bool { return a > b; }
#define CMP_SMALLER [](DCS::f64 a, DCS::f64 b) -> bool { return a < b; }

static Cmpdata waitForAny(DCS::f64* data, size_t start, size_t size, DCS::f64 barrier, DCS::f64 noise_threshold)
{
	Cmpdata cd = Cmpdata();

	if (data[start] > barrier - noise_threshold)
	{
		cd.index = waitForLimit(data, start, size, barrier - noise_threshold, CMP_SMALLER);
		cd.edge = Cmpdata::FALLING;
	}
	else if (data[start] < barrier + noise_threshold)
	{
		cd.index = waitForLimit(data, start, size, barrier + noise_threshold, CMP_LARGER);
		cd.edge = Cmpdata::RISING;
	}

	return cd;
}

// Legacy code
// TODO : Change this in the future from legacy to core functionality only
static std::vector<size_t> countPacketLegacy(DCS::f64 * data, size_t size, DCS::f64 barrier, DCS::f64 threshold)
{
	static DCS::f64 lastDataPoint = barrier + threshold;
	static Cmpdata::Edge lastEdge = Cmpdata::RISING;


	Cmpdata compare;
	compare.index = 0;
	std::vector<size_t> where;
	where.reserve(10); //When real tests are made change this for a likely value

	if (lastDataPoint < barrier - threshold && data[0] > barrier + threshold) [[unlikely]]
	{
		where.push_back(0);
		compare.index = 1;
	}
	else if (lastDataPoint < barrier + threshold && lastDataPoint > barrier - threshold && lastEdge == Cmpdata::RISING) [[unlikely]]
	{
		compare.index = waitForLimit(data, 0, size, barrier + threshold, CMP_LARGER);
		if (compare.index > size)
		{
			lastDataPoint = data[size - 1];
			return where;
		}
		else
		{
			where.push_back(compare.index);
		}
	}

	while (true)
	{
		compare = waitForAny(data, compare.index, size, barrier, threshold);
		lastEdge = compare.edge;
		if (compare.index > size)
		{
			lastDataPoint = data[size - 1];
			return where;
		}
		else if (compare.edge == Cmpdata::RISING)
		{
			where.push_back(compare.index);
		}
	}
}

#define DCS_MATH_USE_LEGACY_COUNTER

DCS::Math::CountResult DCS::Math::countArrayPeak(f64* arr, u64 size, f64 vlo, f64 vhi, f64 vth)
{
    CountResult result;

    #ifdef DCS_MATH_USE_LEGACY_COUNTER
    auto values = countPacketLegacy(arr, size, vlo, vth);
    result.num_detected = values.size();
    result.maximizers = new DCS::u64[result.num_detected]; 
    memcpy(result.maximizers, &values.front(), result.num_detected * sizeof(DCS::u64));
    #else
    // TODO : Create the core counter
    #endif

    return result;
}