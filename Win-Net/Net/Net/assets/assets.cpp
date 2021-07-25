#include "assets.h"

NET_NAMESPACE_BEGIN(Net)
#ifndef BUILD_LINUX
void ShowMessageBox(const char* title, const char* msg, ...)
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vsnprintf(nullptr, 0, msg, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	MessageBoxA(nullptr, str.data(), title, MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

void ShowMessageBox(const wchar_t* title, const wchar_t* msg, ...)
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vswprintf(nullptr, 0, msg, vaArgs);
	std::vector<wchar_t> str(size + 1);
	std::vswprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	MessageBoxW(nullptr, str.data(), title, MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}
#endif

NET_NAMESPACE_BEGIN(Clock)
double GetClockTime()
{
	return (double)clock();
}

double AddTime(const double Time)
{
	// 1000 * Time
	// Example: Time = 1000 * 10.0f -- means 10 seconds
	return (double)clock() + (1000 * Time);
}

void GetTimeW(wchar_t* time)
{
	if (Console::TM_GetTime().tm_hour < 10 && Console::TM_GetTime().tm_min < 10)
		swprintf(time, TIME_LENGTH, CWSTRING("0%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_hour < 10)
		swprintf(time, TIME_LENGTH, CWSTRING("0%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_min < 10)
		swprintf(time, TIME_LENGTH, CWSTRING("%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else
		swprintf(time, TIME_LENGTH, CWSTRING("%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
}

void GetTimeA(char* time)
{
	if (Console::TM_GetTime().tm_hour < 10 && Console::TM_GetTime().tm_min < 10)
		sprintf(time, CSTRING("0%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_hour < 10)
		sprintf(time, CSTRING("0%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_min < 10)
		sprintf(time, CSTRING("%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else
		sprintf(time, CSTRING("%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
}

void GetDateW(wchar_t* date)
{
	if (Console::TM_GetTime().tm_mday < 9)
		swprintf(date, DATE_LENGTH, CWSTRING("0%i-%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else if (Console::TM_GetTime().tm_mon < 9)
		swprintf(date, DATE_LENGTH, CWSTRING("%i-0%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else if (Console::TM_GetTime().tm_mday < 9 && Console::TM_GetTime().tm_mon < 9)
		swprintf(date, DATE_LENGTH, CWSTRING("0%i-0%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else
		swprintf(date, DATE_LENGTH, CWSTRING("%i-%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
}

void GetDateA(char* date)
{
	if (Console::TM_GetTime().tm_mday < 9)
		sprintf(date, CSTRING("0%i-%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else if (Console::TM_GetTime().tm_mon < 9)
		sprintf(date, CSTRING("%i-0%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else if (Console::TM_GetTime().tm_mday < 9 && Console::TM_GetTime().tm_mon < 9)
		sprintf(date, CSTRING("0%i-0%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
	else
		sprintf(date, CSTRING("%i-%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1, Console::TM_GetTime().tm_year + BASE_DATE);
}
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Math)
#if defined(VS13) || defined(BUILD_LINUX)
template <typename TYPE>
TYPE GetRandNumber(TYPE min, TYPE max)
{
	return NULL;
}
#else
template <typename TYPE>
TYPE GetRandNumber(TYPE min, TYPE max)
{
	thread_local static std::mt19937 mt(std::random_device{}());
	thread_local static std::uniform_int_distribution<TYPE> pick;
	return pick(mt, decltype(pick)::param_type{ min, max });
}
#endif
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Random)
void GetRandString(char*& out, const size_t len)
{
	FREE(out);
	out = ALLOC< char >(len + 1);

	for (size_t i = 0; i < len; i++)
		out[i] = CSTRING("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")[Net::Math::GetRandNumber< int >(0, 60)];

	out[len] = '\0';
}

void GetRandStringNew(BYTE*& out, const size_t len)
{
	FREE(out);
	out = ALLOC< BYTE >(len + 1);

	for (size_t i = 0; i < len; i++)
		out[i] = CSTRING("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")[Net::Math::GetRandNumber< int >(0, 60)];

	out[len] = '\0';
}

uintptr_t GetRandSeed()
{
	return Net::Math::GetRandNumber<uintptr_t>(0, UINTPTR_MAX);
}
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Util)
int roundUp(int numToRound, int multiple)
{
	if (multiple == 0)
		return numToRound;

	int remainder = numToRound % multiple;
	if (remainder == 0)
		return numToRound;

	return numToRound + multiple - remainder;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
