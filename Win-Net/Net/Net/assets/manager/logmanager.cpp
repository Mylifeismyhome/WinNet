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

static char __net_output_fname[NET_MAX_PATH] = { 0 };
static bool __net_logging_enabled = false;
void __Net_Enable_Logging()
{
	__net_logging_enabled = true;
	Net::Manager::Log::start();
}

void __Net_Shutdown_Logging()
{
	if (!__net_logging_enabled) return;
	Net::Manager::Log::shutdown();
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

/* MAIN Thread Invoker */
struct [[nodiscard]] __net_logmanager_array_entry_A_t
{
	Net::Console::LogStates state;
	std::string func;
	std::string msg;
	bool save;
};

static std::vector< __net_logmanager_array_entry_A_t> __net__logmanager_holder_a;

struct [[nodiscard]] __net_logmanager_array_entry_W_t
{
	Net::Console::LogStates state;
	std::wstring func;
	std::wstring msg;
	bool save;
};

static std::vector< __net_logmanager_array_entry_W_t> __net__logmanager_holder_w;

/* lock the call - just output one message each lock */
static std::mutex __net_logmanager_critical;
static void __net_logmanager_output_log_a(__net_logmanager_array_entry_A_t entry)
{
	if (entry.msg.empty())
		return;

	char time[TIME_LENGTH];
	Net::Clock::GetTimeA(time);

	char date[DATE_LENGTH];
	Net::Clock::GetDateA(date);

	// display date & time
	auto buffer = ALLOC<char>(TIME_LENGTH + DATE_LENGTH + 3);
	sprintf(buffer, CSTRING("'%s %s'"), date, time);
	buffer[TIME_LENGTH + DATE_LENGTH + 2] = '\0';
	printf(buffer);
	FREE(buffer);

	printf(CSTRING(" "));

	// display prefix in it specific color
	const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
	buffer = ALLOC<char>(prefix.size() + 4);
	sprintf(buffer, CSTRING("[%s]"), prefix.data());
	buffer[prefix.size() + 3] = '\0';

	if (!Net::Console::GetPrintFState())
		SetConsoleOutputColor(Net::Console::GetColorFromState(entry.state));

	printf(buffer);

	FREE(buffer);

	if (!Net::Console::GetPrintFState())
		SetConsoleOutputColor(WHITE);

	// output functionname
	if (!entry.func.empty())
	{
		printf(CSTRING(" "));
		printf(CSTRING("'%s'"), entry.func.data());
	}

	printf(CSTRING(" -> "));

	printf(entry.msg.data());
	printf(CSTRING("\n"));

	// create an entire buffer to save to file
	if (entry.save)
	{
		if (!entry.func.empty())
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + 10;
			buffer = ALLOC<char>(bsize + 1);
			sprintf(buffer, CSTRING("'%s %s' [%s] -> %s\n"), date, time, prefix.data(), entry.msg.data());
			buffer[bsize] = '\0';

			if (OnLogA)
				(*OnLogA)((int)entry.state, buffer);

			auto file = NET_FILEMANAGER(__net_output_fname, NET_FILE_WRITE | NET_FILE_APPAND);
			file.write(buffer);

			FREE(buffer);
		}
		else
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + entry.func.size() + 12;
			auto buffer = ALLOC<char>(bsize + 1);
			sprintf(buffer, CSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), entry.func.data(), entry.msg.data());
			buffer[bsize] = '\0';

			if (OnLogA)
				(*OnLogA)((int)entry.state, buffer);

			auto file = NET_FILEMANAGER(__net_output_fname, NET_FILE_WRITE | NET_FILE_APPAND);
			file.write(buffer);

			FREE(buffer);
		}
	}
	else
	{
		// create entire output for the callback
		if (OnLogA)
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + entry.func.size() + 12;
			auto buffer = ALLOC<char>(bsize + 1);
			sprintf(buffer, CSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), entry.func.data(), entry.msg.data());
			buffer[bsize] = '\0';
			(*OnLogA)((int)entry.state, buffer);
			FREE(buffer);
		}
	}
}

static void __net_logmanager_output_log_w(__net_logmanager_array_entry_W_t entry)
{
	if (entry.msg.empty())
		return;

	wchar_t time[TIME_LENGTH];
	Net::Clock::GetTimeW(time);

	wchar_t date[DATE_LENGTH];
	Net::Clock::GetDateW(date);

	// display date & time
	auto buffer = ALLOC<wchar_t>(TIME_LENGTH + DATE_LENGTH + 3);
	swprintf(buffer, TIME_LENGTH + DATE_LENGTH + 2, CWSTRING("'%s %s'"), date, time);
	buffer[TIME_LENGTH + DATE_LENGTH + 2] = '\0';
	wprintf(buffer);
	FREE(buffer);

	wprintf(CWSTRING(" "));

	// display prefix in it specific color
	const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
	buffer = ALLOC<wchar_t>(prefix.size() + 4);
	swprintf(buffer, prefix.size() + 3, CWSTRING("[%s]"), prefix.data());
	buffer[prefix.size() + 3] = '\0';

	if (!Net::Console::GetPrintFState())
		SetConsoleOutputColor(Net::Console::GetColorFromState(entry.state));

	wprintf(buffer);

	FREE(buffer);

	if (!Net::Console::GetPrintFState())
		SetConsoleOutputColor(WHITE);

	// output functionname
	if (!entry.func.empty())
	{
		wprintf(CWSTRING(" "));
		wprintf(CWSTRING("'%s'"), entry.func.data());
	}

	wprintf(CWSTRING(" -> "));

	wprintf(entry.msg.data());
	wprintf(CWSTRING("\n"));

	// create an entire buffer to save to file
	if (entry.save)
	{
		if (!entry.func.empty())
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + 10;
			auto buffer = ALLOC<wchar_t>(bsize + 1);
			swprintf(buffer, bsize, CWSTRING("'%s %s' [%s] -> %s\n"), date, time, prefix.data(), entry.msg.data());
			buffer[bsize] = '\0';

			if (OnLogW)
				(*OnLogW)((int)entry.state, buffer);

			auto file = NET_FILEMANAGER(__net_output_fname, NET_FILE_WRITE | NET_FILE_APPAND);
			file.write(buffer);

			FREE(buffer);
		}
		else
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + entry.func.size() + 12;
			auto buffer = ALLOC<wchar_t>(bsize + 1);
			swprintf(buffer, bsize, CWSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), entry.func.data(), entry.msg.data());
			buffer[bsize] = '\0';

			if (OnLogW)
				(*OnLogW)((int)entry.state, buffer);

			auto file = NET_FILEMANAGER(__net_output_fname, NET_FILE_WRITE | NET_FILE_APPAND);
			file.write(buffer);

			FREE(buffer);
		}
	}
	else
	{
		// create entire output for the callback
		if (OnLogW)
		{
			const auto prefix = Net::Console::GetLogStatePrefix(entry.state);
			const auto bsize = TIME_LENGTH + DATE_LENGTH + entry.msg.size() + prefix.size() + entry.func.size() + 12;
			auto buffer = ALLOC<wchar_t>(bsize + 1);
			swprintf(buffer, bsize, CWSTRING("'%s %s' [%s] '%s' -> %s\n"), date, time, prefix.data(), entry.func.data(), entry.msg.data());
			buffer[bsize] = '\0';
			(*OnLogW)((int)entry.state, buffer);
			FREE(buffer);
		}
	}
}

static short __net_m_shutdownThread = 0;

static void __net_logmanager_thread()
{
	while (__net_m_shutdownThread != 2)
	{
		/*
		* this scopes will manage the mutex
		*/
		{
			std::lock_guard<std::mutex> guard(__net_logmanager_critical);

			/*
			* print chars
			*/
			while (__net__logmanager_holder_a.size() > 0)
			{
				__net_logmanager_output_log_a(*__net__logmanager_holder_a.begin());
				__net__logmanager_holder_a.erase(__net__logmanager_holder_a.begin());
			}

			/*
			* print wide chars
			*/
			while (__net__logmanager_holder_w.size() > 0)
			{
				__net_logmanager_output_log_w(*__net__logmanager_holder_w.begin());
				__net__logmanager_holder_w.erase(__net__logmanager_holder_w.begin());
			}
		}

#ifdef BUILD_LINUX
		usleep(1 * 1000);
#else
		Kernel32::Sleep(1);
#endif
	}

	__net_m_shutdownThread = 0;
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
			if (!__net_logging_enabled)
				return;

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

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);
				__net_logmanager_array_entry_A_t data;
				data.state = state;
				data.func = std::string(func);
				data.msg = std::string(str.data());
				data.save = false;
				__net__logmanager_holder_a.emplace_back(data);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* funcA, const wchar_t* msg, ...)
		{
			if (!__net_logging_enabled)
				return;

			const size_t lenfunc = strlen(funcA) + 1;
			wchar_t* func = ALLOC<wchar_t>(lenfunc);
			mbstowcs(func, funcA, lenfunc);

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

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);
				__net_logmanager_array_entry_W_t data;
				data.state = state;
				data.func = std::wstring(func);
				data.msg = std::wstring(str.data());
				data.save = false;
				__net__logmanager_holder_w.emplace_back(data);
			}

			FREE(func);
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::String msg)
		{
			if (!__net_logging_enabled)
				return;

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);

				auto ref = msg.get();

				__net_logmanager_array_entry_A_t data;
				data.state = state;
				data.func = std::string(func);
				data.msg = std::string(ref.get());
				data.save = true;
				__net__logmanager_holder_a.emplace_back(data);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::String& msg)
		{
			if (!__net_logging_enabled)
				return;

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);

				auto ref = msg.get();

				__net_logmanager_array_entry_A_t data;
				data.state = state;
				data.func = std::string(func);
				data.msg = std::string(ref.get());
				data.save = true;
				__net__logmanager_holder_a.emplace_back(data);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::ViewString msg)
		{
			if (!__net_logging_enabled)
				return;

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);

				std::string tmp;
				tmp.reserve(msg.size());
				for (size_t i = msg.start(); i < msg.end(); ++i)
				{
					tmp += msg[i];
				}

				__net_logmanager_array_entry_A_t data;
				data.state = state;
				data.func = std::string(func);
				data.msg = tmp;
				data.save = true;
				__net__logmanager_holder_a.emplace_back(data);
			}
		}

		NET_EXPORT_FUNCTION void Log(const LogStates state, const char* func, Net::ViewString& msg)
		{
			if (!__net_logging_enabled)
				return;

			{
				std::lock_guard<std::mutex> guard(__net_logmanager_critical);

				std::string tmp;
				tmp.reserve(msg.size());
				for (size_t i = msg.start(); i < msg.end(); ++i)
				{
					tmp += msg[i];
				}

				__net_logmanager_array_entry_A_t data;
				data.state = state;
				data.func = std::string(func);
				data.msg = tmp;
				data.save = true;
				__net__logmanager_holder_a.emplace_back(data);
			}
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

#ifndef NET_DISABLE_LOGMANAGER
	namespace Manager
	{
		namespace Log
		{
			void start()
			{
				if (__net_m_shutdownThread == 1)
					return;

				std::thread(__net_logmanager_thread).detach();
				__net_m_shutdownThread = 1;
			}

			void shutdown()
			{
				if (__net_m_shutdownThread != 1)
					return;

				__net_m_shutdownThread = 2;

				/*
				* wait for thread to shutdown
				*/
				while (__net_m_shutdownThread != 0)
				{
#ifdef BUILD_LINUX
					usleep(1 * 1000);
#else
					Kernel32::Sleep(1);
#endif
				}
			}

			NET_EXPORT_FUNCTION void SetOutputName(const char* name)
			{
				if (!__net_logging_enabled)
					return;

				Net::String tmp(name);
				if (tmp.find(CSTRING("/")) != NET_STRING_NOT_FOUND
					|| tmp.find(CSTRING("//")) != NET_STRING_NOT_FOUND
					|| tmp.find(CSTRING("\\")) != NET_STRING_NOT_FOUND
					|| tmp.find(CSTRING("\\\\")) != NET_STRING_NOT_FOUND)
				{
					if (tmp.find(CSTRING(":")) != NET_STRING_NOT_FOUND)
					{
						strcpy(__net_output_fname, name);
						strcat(__net_output_fname, CSTRING(".log"));
					}
					else
					{
						strcpy(__net_output_fname, Net::Manager::Directory::homeDir().data());
						strcpy(__net_output_fname, name);
						strcat(__net_output_fname, CSTRING(".log"));
						NET_DIRMANAGER::createDir(__net_output_fname);
					}
				}
				else
				{
					strcpy(__net_output_fname, Net::Manager::Directory::homeDir().data());
					strcat(__net_output_fname, name);
					strcat(__net_output_fname, CSTRING(".log"));
				}
			}

			NET_EXPORT_FUNCTION void SetLogCallbackA(OnLogA_t callback)
			{
				if (!__net_logging_enabled)
					return;

				OnLogA = callback;
			}

			NET_EXPORT_FUNCTION void SetLogCallbackW(OnLogW_t callback)
			{
				if (!__net_logging_enabled)
					return;

				OnLogW = callback;
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* func, const char* msg, ...)
			{
				if (!__net_logging_enabled)
					return;

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

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);
					__net_logmanager_array_entry_A_t data;
					data.state = state;
					data.func = std::string(func);
					data.msg = std::string(str.data());
					data.save = true;
					__net__logmanager_holder_a.emplace_back(data);
				}
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* funcA, const wchar_t* msg, ...)
			{
				if (!__net_logging_enabled)
					return;

				const size_t lenfunc = strlen(funcA) + 1;
				wchar_t* func = ALLOC<wchar_t>(lenfunc);
				mbstowcs(func, funcA, lenfunc);

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

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);
					__net_logmanager_array_entry_W_t data;
					data.state = state;
					data.func = std::wstring(func);
					data.msg = std::wstring(str.data());
					data.save = true;
					__net__logmanager_holder_w.emplace_back(data);
				}

				FREE(func);
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* func, Net::String msg)
			{
				if (!__net_logging_enabled)
					return;

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);

					auto ref = msg.get();

					__net_logmanager_array_entry_A_t data;
					data.state = state;
					data.func = std::string(func);
					data.msg = std::string(ref.get());
					data.save = true;
					__net__logmanager_holder_a.emplace_back(data);
				}
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* func, Net::String& msg)
			{
				if (!__net_logging_enabled)
					return;

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);

					auto ref = msg.get();

					__net_logmanager_array_entry_A_t data;
					data.state = state;
					data.func = std::string(func);
					data.msg = std::string(ref.get());
					data.save = true;
					__net__logmanager_holder_a.emplace_back(data);
				}
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* func, Net::ViewString msg)
			{
				if (!__net_logging_enabled)
					return;

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);

					std::string tmp;
					tmp.reserve(msg.size());
					for (size_t i = msg.start(); i < msg.end(); ++i)
					{
						tmp += msg[i];
					}

					__net_logmanager_array_entry_A_t data;
					data.state = state;
					data.func = std::string(func);
					data.msg = tmp;
					data.save = true;
					__net__logmanager_holder_a.emplace_back(data);
				}
			}

			NET_EXPORT_FUNCTION void Log(const Console::LogStates state, const char* func, Net::ViewString& msg)
			{
				if (!__net_logging_enabled)
					return;

				{
					std::lock_guard<std::mutex> guard(__net_logmanager_critical);

					std::string tmp;
					tmp.reserve(msg.size());
					for (size_t i = msg.start(); i < msg.end(); ++i)
					{
						tmp += msg[i];
					}

					__net_logmanager_array_entry_A_t data;
					data.state = state;
					data.func = std::string(func);
					data.msg = tmp;
					data.save = true;
					__net__logmanager_holder_a.emplace_back(data);
				}
			}
		}
	}
#endif
}