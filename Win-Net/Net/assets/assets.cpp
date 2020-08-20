#include "assets.h"

NET_NAMESPACE_BEGIN(Net)
void ShowMessageBox(const char* title, const char* msg)
{
	MessageBoxA(nullptr, msg, title, MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

void ShowMessageBox(const wchar_t* title, const wchar_t* msg)
{
	MessageBoxW(nullptr, msg, title, MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

NET_NAMESPACE_BEGIN(Console)
static std::vector<LogInformations> vLogs;
static bool DisablePrintF = false;

tm TM_GetTime()
{
	auto timeinfo = tm();
#ifdef _WIN64
	auto t = _time64(nullptr);   // get time now
	_localtime64_s(&timeinfo, &t);
#else
	auto t = _time32(nullptr);   // get time now
	_localtime32_s(&timeinfo, &t);
#endif
	return timeinfo;
}

void Log(const LogStates state, const char* func, const char* msg, ...)
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vsnprintf(nullptr, 0, msg, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	if (strcmp(func, CSTRING("")) == 0)
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + 9;
		auto displayBuff = ALLOC<char>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		sprintf_s(displayBuff, DisplaySize, CSTRING("[%s] %s\n"), time, str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState(state));
			printf_s(displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		LogInformations logInfo;
		logInfo.buffer = ALLOC<char>((DisplaySize - 2) + 1);
		memcpy(logInfo.buffer, displayBuff, DisplaySize - 2);
		logInfo.buffer[DisplaySize - 2] = '\0';
		logInfo.state = state;
		InsertLog(logInfo);

		FREE(displayBuff);
	}
	else
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + strlen(func) + 11;
		auto displayBuff = ALLOC<char>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		sprintf_s(displayBuff, DisplaySize, CSTRING("[%s][%s] %s\n"), time, func, str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState(state));
			printf_s(displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		LogInformations logInfo;
		logInfo.buffer = ALLOC<char>((DisplaySize - 2) + 1);
		memcpy(logInfo.buffer, displayBuff, DisplaySize - 2);
		logInfo.buffer[DisplaySize - 2] = '\0';
		logInfo.state = state;
		InsertLog(logInfo);

		FREE(displayBuff);
	}
}

void Log(const LogStates state, const char* func, const wchar_t* msg, ...)
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vswprintf(nullptr, 0, msg, vaArgs);
	std::vector<wchar_t> str(size + 1);
	std::vswprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	if (strcmp(func, CSTRING("")) == 0)
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + 9;
		auto displayBuff = ALLOC<wchar_t>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		swprintf_s(displayBuff, DisplaySize, CWSTRING(L"[%s] %s\n"), reinterpret_cast<const wchar_t*>(time), str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState(state));
			wprintf_s(displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		LogInformations logInfo;
		logInfo.wbuffer = ALLOC<wchar_t>((DisplaySize - 2) + 1);
		memcpy(logInfo.wbuffer, displayBuff, DisplaySize - 2);
		logInfo.wbuffer[DisplaySize - 2] = '\0';
		logInfo.state = state;
		InsertLog(logInfo);

		FREE(displayBuff);
	}
	else
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + strlen(func) + 11;
		auto displayBuff = ALLOC<wchar_t>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		swprintf_s(displayBuff, DisplaySize, CWSTRING(L"[%s][%s] %s\n"), reinterpret_cast<const wchar_t*>(time), reinterpret_cast<const wchar_t*>(func), str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState(state));
			wprintf_s(displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		LogInformations logInfo;
		logInfo.wbuffer = ALLOC<wchar_t>((DisplaySize - 2) + 1);
		memcpy(logInfo.wbuffer, displayBuff, DisplaySize - 2);
		logInfo.wbuffer[DisplaySize - 2] = '\0';
		logInfo.state = state;
		InsertLog(logInfo);

		FREE(displayBuff);
	}
}

void ChangeStdOutputColor(const int Color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color);
}

void InsertLog(LogInformations& logInfo)
{
	vLogs.push_back(logInfo);
}

std::vector<LogInformations>& GetLogs()
{
	return vLogs;
}

void ClearLogs()
{
	for(auto& entry : vLogs)
	{
		FREE(entry.buffer);
		FREE(entry.wbuffer);
	}
	
	vLogs.clear();
}

void SetPrintF(const bool state)
{
	DisablePrintF = !state;
}

bool GetPrintFState()
{
	return DisablePrintF;
}

WORD GetColorFromState(const LogStates state)
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
NET_NAMESPACE_END

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

void GetTime(char*& time)
{
	FREE(time);
	const size_t size = 6;
	time = ALLOC<char>(size + 1);
	if (Console::TM_GetTime().tm_hour < 10 && Console::TM_GetTime().tm_min < 10)
		sprintf_s(time, size, CSTRING("0%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_hour < 10)
		sprintf_s(time, size, CSTRING("0%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else if (Console::TM_GetTime().tm_min < 10)
		sprintf_s(time, size, CSTRING("%i:0%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
	else
		sprintf_s(time, size, CSTRING("%i:%i"), Console::TM_GetTime().tm_hour, Console::TM_GetTime().tm_min);
}

void GetDate(char*& date)
{
	FREE(date);
	const size_t size = 6;
	date = ALLOC<char>(size + 1);
	if (Console::TM_GetTime().tm_mday < 10)
		sprintf_s(date, size, CSTRING("0%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1);
	else if (Console::TM_GetTime().tm_mon < 10)
		sprintf_s(date, size, CSTRING("%i-0%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1);
	else if (Console::TM_GetTime().tm_mday < 10 && Console::TM_GetTime().tm_mon < 10)
		sprintf_s(date, size, CSTRING("0%i-0%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1);
	else
		sprintf_s(date, size, CSTRING("%i-%i"), Console::TM_GetTime().tm_mday, Console::TM_GetTime().tm_mon + 1);
}
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Keyboard)
const char* GetKeyName(const int Key)
{
	switch (Key)
	{
	case Key::SLEEP:
		return "SLEEP";

	case Key::NUMPAD0:
		return "NUMPAD0";

	case Key::NUMPAD1:
		return "NUMPAD1";

	case Key::NUMPAD2:
		return "NUMPAD2";

	case Key::NUMPAD3:
		return "NUMPAD3";

	case Key::NUMPAD4:
		return "NUMPAD4";

	case Key::NUMPAD5:
		return "NUMPAD5";

	case Key::NUMPAD6:
		return "NUMPAD6";

	case Key::NUMPAD7:
		return "NUMPAD7";

	case Key::NUMPAD8:
		return "NUMPAD8";

	case Key::NUMPAD9:
		return "NUMPAD9";

	case Key::MULTIPLY:
		return "MULTIPLY";

	case Key::ADD:
		return "ADD";

	case Key::SEPARATOR:
		return "SEPARATOR";

	case Key::SUBTRACT:
		return "SUBTRACT";

	case Key::DECIMAL:
		return "DECIMAL";

	case Key::DIVIDE:
		return "DIVIDE";

	case Key::F1:
		return "F1";

	case Key::F2:
		return "F2";

	case Key::F3:
		return "F3";

	case Key::F4:
		return "F4";

	case Key::F5:
		return "F5";

	case Key::F6:
		return "F6";

	case Key::F7:
		return "F7";

	case Key::F8:
		return "F8";

	case Key::F9:
		return "F9";

	case Key::F10:
		return "F10";

	case Key::F11:
		return "F11";

	case Key::F12:
		return "F12";

	case Key::F13:
		return "F13";

	case Key::F14:
		return "F14";

	case Key::F15:
		return "F15";

	case Key::F16:
		return "F16";

	case Key::F17:
		return "F17";

	case Key::F18:
		return "F18";

	case Key::F19:
		return "F19";

	case Key::F20:
		return "F20";

	case Key::F21:
		return "F21";

	case Key::F22:
		return "F22";

	case Key::F23:
		return "F23";

	case Key::F24:
		return "F24";

	case Key::NAVIGATION_VIEW:
		return "NAVIGATION_VIEW";

	case Key::NAVIGATION_MENU:
		return "NAVIGATION_MENU";

	case Key::NAVIGATION_UP:
		return "NAVIGATION_UP";

	case Key::NAVIGATION_DOWN:
		return "NAVIGATION_DOWN";

	case Key::NAVIGATION_LEFT:
		return "NAVIGATION_LEFT";

	case Key::NAVIGATION_RIGHT:
		return "NAVIGATION_RIGHT";

	case Key::NAVIGATION_ACCEPT:
		return "NAVIGATION_ACCEPT";

	case Key::NAVIGATION_CANCEL:
		return "NAVIGATION_CANCEL";

	case Key::NUMLOCK:
		return "NUMLOCK";

	case Key::SCROLL:
		return "SCROLL";

	case Key::OEM_NEC_EQUAL:
		return "OEM_NEC_EQUAL";

	case Key::OEM_FJ_JISHO:
		return "OEM_FJ_JISHO";

	case Key::OEM_FJ_MASSHOU:
		return "OEM_FJ_MASSHOU";

	case Key::OEM_FJ_TOUROKU:
		return "OEM_FJ_TOUROKU";

	case Key::OEM_FJ_LOYA:
		return "OEM_FJ_LOYA";

	case Key::LSHIFT:
		return "LSHIFT";

	case Key::RSHIFT:
		return "RSHIFT";

	case Key::LCONTROL:
		return "LCONTROL";

	case Key::RCONTROL:
		return "RCONTROL";

	case Key::LMENU:
		return "LMENU";

	case Key::RMENU:
		return "RMENU";

	case Key::BROWSER_BACK:
		return "BROWSER_BACK";

	case Key::BROWSER_FORWARD:
		return "BROWSER_FORWARD";

	case Key::BROWSER_REFRESH:
		return "BROWSER_REFRESH";

	case Key::BROWSER_STOP:
		return "BROWSER_STOP";

	case Key::BROWSER_SEARCH:
		return "BROWSER_SEARCH";

	case Key::BROWSER_FAVORITES:
		return "BROWSER_FAVORITES";

	case Key::BROWSER_HOME:
		return "BROWSER_HOME";

	case Key::VOLUME_MUTE:
		return "VOLUME_MUTE";

	case Key::VOLUME_DOWN:
		return "VOLUME_DOWN";

	case Key::VOLUME_UP:
		return "VOLUME_UP";

	case Key::MEDIA_NEXT_TRACK:
		return "MEDIA_NEXT_TRACK";

	case Key::MEDIA_PREV_TRACK:
		return "MEDIA_PREV_TRACK";

	case Key::MEDIA_STOP:
		return "MEDIA_STOP";

	case Key::MEDIA_PLAY_PAUSE:
		return "MEDIA_PLAY_PAUSE";

	case Key::LAUNCH_MAIL:
		return "LAUNCH_MAIL";

	case Key::LAUNCH_MEDIA_SELECT:
		return "LAUNCH_MEDIA_SELECT";

	case Key::LAUNCH_APP1:
		return "LAUNCH_APP1";

	case Key::LAUNCH_APP2:
		return "LAUNCH_APP2";

	case Key::OEM_1:
		return "OEM_1";

	case Key::OEM_PLUS:
		return "OEM_PLUS";

	case Key::OEM_COMMA:
		return "OEM_COMMA";

	case Key::OEM_MINUS:
		return "OEM_MINUS";

	case Key::OEM_PERIOD:
		return "OEM_PERIOD";

	case Key::OEM_2:
		return "OEM_2";

	case Key::OEM_3:
		return "OEM_3";

	case Key::OEM_4:
		return "OEM_4";

	case Key::OEM_5:
		return "OEM_5";

	case Key::OEM_6:
		return "OEM_6";

	case Key::OEM_7:
		return "OEM_7";

	case Key::OEM_8:
		return "OEM_8";

	default:
		break;
	}

	return "";
}

bool KeyWasPressed(const int vKey)
{
	static auto wPressed = false;
	static auto isPressed = false;
	static auto oldvKey = -1;

	//if (GetActiveWindow() == GetForegroundWindow())
	{
		if (oldvKey != -1)
		{
			if (oldvKey != vKey && isPressed)
				return false;
		}

		if (GetAsyncKeyState(vKey))
		{
			isPressed = true;
			oldvKey = vKey;
		}

		if (!wPressed && isPressed)
		{
			if (!GetAsyncKeyState(vKey))
			{
				wPressed = true;
			}
		}
		else if (wPressed && isPressed)
		{
			wPressed = false;
			isPressed = false;
		}

		return wPressed;
	}
}

bool KeyIsPressed(const int vKey)
{
	static auto isPressed = false;
	static auto oldvKey = -1;

	if (GetActiveWindow() == GetForegroundWindow())
	{
		if (oldvKey != -1)
		{
			if (oldvKey != vKey && isPressed)
				return false;
		}

		if (GetAsyncKeyState(vKey))
		{
			isPressed = true;
			oldvKey = vKey;
		}

		if (isPressed && !GetAsyncKeyState(vKey))
			isPressed = false;

		return isPressed;
	}
	else
		return NULL;
}

bool KeyIsPressed2(const int vKey)
{
	static auto isPressed = false;
	static auto FirstPressed = false;
	TIMER SpamTimer = CURRENTCLOCKTIME;

	if (GetActiveWindow() == GetForegroundWindow())
	{
		if (SpamTimer < CURRENTCLOCKTIME)
		{
			if (isPressed && !GetAsyncKeyState(vKey))
			{
				FirstPressed = false;
				isPressed = false;
			}
			else if (GetAsyncKeyState(vKey))
			{
				isPressed = true;

				SpamTimer = CREATETIMER(FirstPressed ? 0.075f : 0.5f);
				FirstPressed = true;
			}

			return isPressed;
		}

		return false;
	}
	else
		return NULL;
}
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Math)
template <typename TYPE>
TYPE GetRandNumber(TYPE min, TYPE max)
{
	thread_local static std::mt19937 mt(std::random_device{}());
	thread_local static std::uniform_int_distribution<TYPE> pick;
	return pick(mt, decltype(pick)::param_type{ min, max });
}
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Random)
void GetRandString(char** out, const size_t len)
{
	if (*out)
		FREE(*out);

	if (!*out)
		*out = ALLOC< char >(len + 1);

	const auto charset = CSTRING("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

	for (size_t i = 0; i < len; i++)
		out[0][i] = charset[Net::Math::GetRandNumber< int >(0, 60)];

	out[0][len] = '\0';
}

void GetRandStringNew(BYTE** out, const size_t len)
{
	if (*out)
		FREE(*out);

	if (!*out)
		*out = ALLOC< BYTE >(len + 1);

	const auto charset = CSTRING("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

	for (size_t i = 0; i < len; i++)
		out[0][i] = charset[Net::Math::GetRandNumber< int >(0, 60)];

	out[0][len] = '\0';
}

uintptr_t GetRandSeed()
{
	return Net::Math::GetRandNumber<uintptr_t>(0, UINTPTR_MAX);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
