/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "logmanager.h"
#include <mutex>

#include <Net/assets/assets.h>
#include <Net/assets/manager/filemanager.h>
#include <Net/Import/Kernel32.hpp>

#ifndef NET_DISABLE_LOGMANAGER
// Color codes
CONSTEXPR auto BLACK = 0;
CONSTEXPR auto BLUE = 1;
CONSTEXPR auto GREEN = 2;
CONSTEXPR auto CYAN = 3;
CONSTEXPR auto RED = 4;
CONSTEXPR auto MAGENTA = 5;
CONSTEXPR auto BROWN = 6;
CONSTEXPR auto LIGHTGRAY = 7;
CONSTEXPR auto DARKGRAY = 8;
CONSTEXPR auto LIGHTBLUE = 9;
CONSTEXPR auto LIGHTGREEN = 10;
CONSTEXPR auto LIGHTCYAN = 11;
CONSTEXPR auto LIGHTRED = 12;
CONSTEXPR auto LIGHTMAGENTA = 13;
CONSTEXPR auto YELLOW = 14;
CONSTEXPR auto WHITE = 15;

static BYTE __net_logging_enabled = 0;
void __Net_Enable_Logging()
{
	__net_logging_enabled = 1;
}

// global override able callback
static void (*OnLogA)(int state, const char* buffer) = nullptr;
static void (*OnLogW)(int state, const wchar_t* buffer) = nullptr;

static void SetConsoleOutputColor(const int color)
{
#ifndef BUILD_LINUX
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
}

#endif
namespace Net
{
	namespace Console
	{
#ifndef NET_DISABLE_LOGMANAGER
		static bool DisablePrintF = false;
#endif

		NET_EXPORT_FUNCTION tm TM_GetTime()
		{
			auto timeinfo = tm();
#ifdef BUILD_LINUX
			time_t tm = time(nullptr);
			auto p_timeinfo = localtime(&tm);
			timeinfo = *p_timeinfo;
#else
#ifdef _WIN64
			auto t = _time64(nullptr);   // get time now
			_localtime64_s(&timeinfo, &t);
#else
			auto t = _time32(nullptr);   // get time now
			_localtime32_s(&timeinfo, &t);
#endif
#endif
			return timeinfo;
		}

#ifndef NET_DISABLE_LOGMANAGER
		NET_EXPORT_FUNCTION void SetLogCallbackA(OnLogA_t callback)
		{
			if (__net_logging_enabled == 0)
			{
				return;
			}

			OnLogA = callback;
		}

		NET_EXPORT_FUNCTION void SetLogCallbackW(OnLogW_t callback)
		{
			if (__net_logging_enabled == 0)
			{
				return;
			}

			OnLogW = callback;
		}

		NET_EXPORT_FUNCTION std::string GetLogStatePrefix(LogStates state)
		{
			switch (static_cast<int>(state))
			{
			case (int)LogStates::debug:
				return std::string(CSTRING("DEBUG"));

			case (int)LogStates::warning:
				return std::string(CSTRING("WARNING"));

			case (int)LogStates::error:
				return std::string(CSTRING("ERROR"));

			case (int)LogStates::success:
				return std::string(CSTRING("SUCCESS"));

			case (int)LogStates::peer:
				return std::string(CSTRING("PEER"));

			default:
				return std::string(CSTRING("INFO"));
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, const char* msg, ...)
		{
			if (__net_logging_enabled == 0)
			{
				return;
			}

			va_list vaArgs;

#ifdef BUILD_LINUX
			va_start(vaArgs, msg);
			const size_t size = std::vsnprintf(nullptr, 0, msg, vaArgs);
			va_end(vaArgs);

			va_start(vaArgs, msg);
			std::vector<char> str(size + 1);
			std::vsnprintf(str.data(), str.size(), msg, vaArgs);
			va_end(vaArgs);
#else
			va_start(vaArgs, msg);
			const size_t size = std::vsnprintf(nullptr, 0, msg, vaArgs);
			std::vector<char> str(size + 1);
			std::vsnprintf(str.data(), str.size(), msg, vaArgs);
			va_end(vaArgs);
#endif

			char time[TIME_LENGTH];
			Net::Clock::GetTimeA(time);

			char date[DATE_LENGTH];
			Net::Clock::GetDateA(date);

			// display date & time
			auto buffer = ALLOC<char>(TIME_LENGTH + DATE_LENGTH + 3);
			sprintf(buffer, CSTRING("'%s %s'"), date, time);
			buffer[TIME_LENGTH + DATE_LENGTH + 2] = '\0';
			printf(buffer);
			FREE<char>(buffer);

			printf(CSTRING(" "));

			// display prefix in it specific color
			const auto prefix = Net::Console::GetLogStatePrefix(state);
			buffer = ALLOC<char>(prefix.size() + 4);
			sprintf(buffer, CSTRING("[%s]"), prefix.data());
			buffer[prefix.size() + 3] = '\0';

			if (Net::Console::GetPrintFState() == 0)
			{
				SetConsoleOutputColor(Net::Console::GetColorFromState(state));
			}

			printf(buffer);

			FREE<char>(buffer);

			if (Net::Console::GetPrintFState() == 0)
			{
				SetConsoleOutputColor(WHITE);
			}

			// output functionname
			if (func[0] != '0')
			{
				printf(CSTRING(" "));
				printf(CSTRING("'%s'"), func);
			}

			printf(CSTRING(" -> "));

			printf(str.data());
			printf(CSTRING("\n"));

			// create entire output for the callback
			if (OnLogA)
			{
				const auto prefix = Net::Console::GetLogStatePrefix(state);
				const auto bsize = TIME_LENGTH + DATE_LENGTH + str.size() + prefix.size() + strlen(func) + 12;
				auto buffer = ALLOC<char>(bsize + 1);
				sprintf(buffer, CSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), func, str.data());
				buffer[bsize] = '\0';
				(*OnLogA)((int)state, buffer);
				FREE<char>(buffer);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* funcA, const wchar_t* msg, ...)
		{
			if (__net_logging_enabled == 0)
			{
				return;
			}

			va_list vaArgs;

#ifdef BUILD_LINUX
			va_start(vaArgs, msg);
			const size_t size = std::vswprintf(nullptr, 0, msg, vaArgs);
			va_end(vaArgs);

			va_start(vaArgs, msg);
			std::vector<wchar_t> str(size + 1);
			std::vswprintf(str.data(), str.size(), msg, vaArgs);
			va_end(vaArgs);
#else
			va_start(vaArgs, msg);
			const size_t size = std::vswprintf(nullptr, 0, msg, vaArgs);
			std::vector<wchar_t> str(size + 1);
			std::vswprintf(str.data(), str.size(), msg, vaArgs);
			va_end(vaArgs);
#endif

			const size_t lenfunc = strlen(funcA) + 1;
			wchar_t* func = ALLOC<wchar_t>(lenfunc);
			mbstowcs(func, funcA, lenfunc);

			wchar_t time[TIME_LENGTH];
			Net::Clock::GetTimeW(time);

			wchar_t date[DATE_LENGTH];
			Net::Clock::GetDateW(date);

			// display date & time
			auto buffer = ALLOC<wchar_t>(TIME_LENGTH + DATE_LENGTH + 3);
			swprintf(buffer, TIME_LENGTH + DATE_LENGTH + 2, CWSTRING("'%s %s'"), date, time);
			buffer[TIME_LENGTH + DATE_LENGTH + 2] = '\0';
			wprintf(buffer);
			FREE<wchar_t>(buffer);

			wprintf(CWSTRING(" "));

			// display prefix in it specific color
			const auto prefix = Net::Console::GetLogStatePrefix(state);
			buffer = ALLOC<wchar_t>(prefix.size() + 4);
			swprintf(buffer, prefix.size() + 3, CWSTRING("[%s]"), prefix.data());
			buffer[prefix.size() + 3] = '\0';

			if (Net::Console::GetPrintFState() == 0)
			{
				SetConsoleOutputColor(Net::Console::GetColorFromState(state));
			}

			wprintf(buffer);

			FREE<wchar_t>(buffer);

			if (Net::Console::GetPrintFState() == 0)
			{
				SetConsoleOutputColor(WHITE);
			}

			// output functionname
			if (func[0] != '0')
			{
				wprintf(CWSTRING(" "));
				wprintf(CWSTRING("'%s'"), func);
			}

			wprintf(CWSTRING(" -> "));

			wprintf(str.data());
			wprintf(CWSTRING("\n"));

			// create entire output for the callback
			if (OnLogW)
			{
				const auto prefix = Net::Console::GetLogStatePrefix(state);
				const auto bsize = TIME_LENGTH + DATE_LENGTH + str.size() + prefix.size() + wcslen(func) + 12;
				auto buffer = ALLOC<wchar_t>(bsize + 1);
				swprintf(buffer, bsize, CWSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), func, str.data());
				buffer[bsize] = '\0';
				(*OnLogW)((int)state, buffer);
				FREE<wchar_t>(buffer);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::String msg)
		{
			auto ref = msg.data();
			Log(state, func, ref.data());
			return;
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::String& msg)
		{
			auto ref = msg.data();
			Log(state, func, ref.data());
			return;
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::ViewString msg)
		{
			std::string tmp;
			tmp.reserve(msg.size());
			for (size_t i = msg.start(); i < msg.end(); ++i)
			{
				tmp += msg[i];
			}

			Log(state, func, tmp.data());
			return;
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::ViewString& msg)
		{
			std::string tmp;
			tmp.reserve(msg.size());
			for (size_t i = msg.start(); i < msg.end(); ++i)
			{
				tmp += msg[i];
			}

			Log(state, func, tmp.data());
			return;
		}

		NET_EXPORT_FUNCTION void SetPrintF(const bool state)
		{
			DisablePrintF = !state;
		}

		NET_EXPORT_FUNCTION bool GetPrintFState()
		{
			return DisablePrintF;
		}

		NET_EXPORT_FUNCTION WORD GetColorFromState(const LogStates state)
		{
			switch (state)
			{
			case LogStates::normal:
				return WHITE;

			case LogStates::debug:
				return LIGHTBLUE;

			case LogStates::warning:
				return MAGENTA;

			case LogStates::error:
				return LIGHTRED;

			case LogStates::success:
				return LIGHTGREEN;

			case LogStates::peer:
				return YELLOW;

			default:
				return WHITE;
			}
		}
#endif
	}
}