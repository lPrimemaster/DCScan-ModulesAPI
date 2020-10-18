#include "../include/DCS_ModuleUtils.h"
#include "../include/internal.h"
#include <string>

const DCS::Utils::String DCS::Timer::Timestamp::to_string() const
{
	std::string ts;
	auto add_value = [&](std::string t, i64 v)
	{
		if (v > 0) ts += std::to_string(v) + t;
	};

	add_value("h ", hour);
	add_value("m ", min);
	add_value("s ", sec);
	add_value("ms ", millis);
	add_value("us ", micros);
	add_value("ns", nanos);

	return Utils::String(ts.c_str());
}

DCS::Timer::SystemTimer DCS::Timer::New()
{
	auto now = std::chrono::steady_clock::now();
	GenericHandle hnd = AllocateGenericHandle(sizeof(now), &now);
	auto st = SystemTimer();
	st.point = hnd;
	return st;
}

void DCS::Timer::Delete(SystemTimer timer)
{
	FreeGenericHandle(timer.point);
}

DCS::Timer::Timestamp DCS::Timer::GetTimestamp(SystemTimer timer)
{
	auto now = std::chrono::steady_clock::now();
	auto ns = now - *reinterpret_cast<std::chrono::time_point<std::chrono::steady_clock>*>(timer.point);

	auto h = std::chrono::duration_cast<std::chrono::hours>(ns);
	auto m = std::chrono::duration_cast<std::chrono::minutes>(ns);
	auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);

	auto add_value = [&](i64 v) ->i16
	{
		if (v > 0) return (i16)v;
		else return 0;
	};

	Timestamp ts;

	ts.hour = add_value(h.count());
	ts.min = add_value((m - h).count());
	ts.sec = add_value((s - m - h).count());
	ts.millis = add_value((ms - s - m - h).count());
	ts.micros = add_value((us - ms - s - m - h).count());
	ts.nanos = add_value((ns - us - ms - s - m - h).count());

	return ts;
}

DCS::Utils::String DCS::Timer::GetTimestampString(SystemTimer timer)
{
	auto now = std::chrono::steady_clock::now();
	auto ns = now - *reinterpret_cast<std::chrono::time_point<std::chrono::steady_clock>*>(timer.point);

	std::string ts;

	auto h = std::chrono::duration_cast<std::chrono::hours>(ns);
	auto m = std::chrono::duration_cast<std::chrono::minutes>(ns);
	auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);

	auto add_value = [&](std::string t, i64 v)
	{
		if(v > 0) ts += std::to_string(v) + t;
	};

	add_value("h ", h.count());
	add_value("m ", (m - h).count());
	add_value("s ", (s - m - h).count());
	add_value("ms ", (ms - s - m - h).count());
	add_value("us ", (us - ms - s - m - h).count());
	add_value("ns", (ns - us - ms - s - m - h).count());

	return DCS::Utils::String(ts.c_str());
}

DCS::i64 DCS::Timer::GetNanoseconds(SystemTimer timer)
{
	auto now = std::chrono::steady_clock::now();
	return (now - *reinterpret_cast<std::chrono::time_point<std::chrono::steady_clock>*>(timer.point)).count();
}
