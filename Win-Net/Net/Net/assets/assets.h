#pragma once
#include <Net/Net/Net.h>
#include <Net/assets/manager/dirmanager.h>
#include <Net/Cryption/XOR.h>

NET_DSA_BEGIN

CONSTEXPR auto BASE_DATE = 1900;

#define TIME_LENGTH 6
#define TIME_LEN TIME_LENGTH
#define DATE_LENGTH 11
#define DATE_LEN DATE_LENGTH

#ifndef BUILD_LINUX
// Messagebox
#define SHOW_MESSAGEBOX(msg, ...) Net::ShowMessageBox("", msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_ERROR(msg, ...) Net::ShowMessageBox(CSTRING("ERROR"), msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_SUCCESS(msg, ...) Net::ShowMessageBox(CSTRING("SUCCESS"), msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_DEBUG(msg, ...) Net::ShowMessageBox(CSTRING("DEBUG"), msg, __VA_ARGS__);
#endif

// Clock
#define CURRENTCLOCKTIME Net::Clock::GetClockTime()
#define CREATETIMER(x) Net::Clock::AddTime(x)

#ifdef UNICODE
#define CURRENTTIME Net::Clock::GetTimeW
#define CURRENTDATE Net::Clock::GetDateW
#else
#define CURRENTTIME Net::Clock::GetTimeA
#define CURRENTDATE Net::Clock::GetDateA
#endif

// Keyboard
#define KEYWASPRESSED(x) Net::Keyboard::KeyWasPressed(x)
#define KEYISPRESSED(x) Net::Keyboard::KeyIsPressed(x)
#define KEYISPRESSED2(x) Net::Keyboard::KeyIsPressed2(x)
#define KEYBOARD Net::Keyboard::Key
#define GET_KEYBOARD_KEYNAME(x) Net::Keyboard::GetKeyName(x)

// Console
#define CONSOLE Net::Console
#define DISABLE_PRINTF Net::Console::SetPrintF(false);
#define ENABLE_PRINTF Net::Console::SetPrintF(true);

// Time
#define TIMETABLE struct tm

namespace Net
{
#ifndef BUILD_LINUX
	void ShowMessageBox(const char*, const char*, ...);
	void ShowMessageBox(const wchar_t*, const wchar_t*, ...);
#endif

	namespace Clock
	{
		double GetClockTime();
		double AddTime(double);
		void GetTimeW(wchar_t*);
		void GetTimeA(char*);
		void GetDateW(wchar_t*);
		void GetDateA(char*);
	}

	namespace Math
	{
		int GetRandNumber(int min, int max);
	}

	namespace Random
	{
		NET_EXPORT_FUNCTION void GetRandString(char*& out, size_t len);
		NET_EXPORT_FUNCTION void GetRandStringNew(BYTE*& out, size_t len);
		NET_EXPORT_FUNCTION int GetRandSeed();
	}

	namespace Util
	{
		int roundUp(int, int);
	}
}
NET_DSA_END