#pragma once
#include <Net/Net.h>
#include <assets/manager/filemanager.h>
#include <assets/manager/dirmanager.h>

// Log
#include <assets/manager/logmanager.h>

#include <random>

#define DEFAULT_LOG_LIMIT 1000

// Messagebox
#define SHOW_MESSAGEBOX(...) Net::ShowMessageBox("", __VA_ARGS__);
#define SHOW_MESSAGEBOX_ERROR(...) Net::ShowMessageBox("ERROR", __VA_ARGS__);
#define SHOW_MESSAGEBOX_SUCCESS(...) Net::ShowMessageBox("SUCCESS", __VA_ARGS__);
#define SHOW_MESSAGEBOX_DEBUG(...) Net::ShowMessageBox("DEBUG", __VA_ARGS__);

// Clock
#define CURRENTCLOCKTIME Net::Clock::GetClockTime()
#define CREATETIMER(x) Net::Clock::AddTime(x)
#define CURRENTTIME Net::Clock::GetTime
#define CURRENTDATE Net::Clock::GetDate

#define TIMER static double

// Keyboard
#define KEYWASPRESSED(x) Net::Keyboard::KeyWasPressed(x)
#define KEYISPRESSED(x) Net::Keyboard::KeyIsPressed(x)
#define KEYISPRESSED2(x) Net::Keyboard::KeyIsPressed2(x)
#define KEYBOARD Net::Keyboard::Key
#define GET_KEYBOARD_KEYNAME(x) Net::Keyboard::GetKeyName(x)
#define SETKEYBOARDFOCUS(x) Net::Keyboard::SetKeyBoardFocus(x)
#define GETKEYBOARDFOCUS Net::Keyboard::GetKeyBoardFocus()

// Console
#define CONSOLE Net::Console
#define DISABLE_PRINTF Net::Console::SetPrintF(false);
#define ENABLE_PRINTF Net::Console::SetPrintF(true);

// Time
#define TIMETABLE struct tm

BEGIN_NAMESPACE(Net)
void ShowMessageBox(const char*, const char*);
void ShowMessageBox(const wchar_t*, const wchar_t*);

BEGIN_NAMESPACE(Console)
enum class LogStates
{
	normal = 0,
	debug,
	warning,
	error,
	success,
	peer
};

struct LogInformations
{
	explicit operator bool() const
	{
		return (buffer ? true : wbuffer ? true : false);
	}

	char* buffer; // msg
	wchar_t* wbuffer; // wmsg
	LogStates state; // debug, warning, peer, error, success

	LogInformations()
	{
		buffer = nullptr;
		wbuffer = nullptr;
		state = LogStates::normal;
	}
};

extern "C" NET_API tm TM_GetTime();
void Log(LogStates, const char*, const char*, ...);
void Log(LogStates, const char*, const wchar_t*, ...);
extern "C" NET_API void ChangeStdOutputColor(int);

extern "C" NET_API void InsertLog(LogInformations&);
extern "C" NET_API LogInformations* GetLogs();
extern "C" NET_API void SetLogLimit(UINT);
extern "C" NET_API unsigned int GetLogSize();

extern "C" NET_API void ClearLogs();

extern "C" NET_API void SetPrintF(bool);
extern "C" NET_API bool GetPrintFState();
extern "C" NET_API WORD GetColorFromState(LogStates);
END_NAMESPACE

BEGIN_NAMESPACE(Clock)
extern "C" NET_API double GetClockTime();
extern "C" NET_API double AddTime(double);
extern "C" NET_API void GetTime(char*&);
extern "C" NET_API void GetDate(char*&);
END_NAMESPACE

BEGIN_NAMESPACE(Keyboard)
enum Key
{
	SLEEP = 0x5F,
	NUMPAD0,
	NUMPAD1,
	NUMPAD2,
	NUMPAD3,
	NUMPAD4,
	NUMPAD5,
	NUMPAD6,
	NUMPAD7,
	NUMPAD8,
	NUMPAD9,
	MULTIPLY,
	ADD,
	SEPARATOR,
	SUBTRACT,
	DECIMAL,
	DIVIDE,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
	NAVIGATION_VIEW,
	NAVIGATION_MENU,
	NAVIGATION_UP,
	NAVIGATION_DOWN,
	NAVIGATION_LEFT,
	NAVIGATION_RIGHT,
	NAVIGATION_ACCEPT,
	NAVIGATION_CANCEL,
	NUMLOCK,
	SCROLL,
	OEM_NEC_EQUAL, // '=' key on numpad
	OEM_FJ_JISHO, // 'Dictionary' key
	OEM_FJ_MASSHOU, // 'Unregister word' key
	OEM_FJ_TOUROKU, // 'Register word' key
	OEM_FJ_LOYA, // 'Left OYAYUBI' key
	OEM_FJ_ROYA, // 'Right OYAYUBI' key
	LSHIFT,
	RSHIFT,
	LCONTROL,
	RCONTROL,
	LMENU,
	RMENU,
	BROWSER_BACK,
	BROWSER_FORWARD,
	BROWSER_REFRESH,
	BROWSER_STOP,
	BROWSER_SEARCH,
	BROWSER_FAVORITES,
	BROWSER_HOME,
	VOLUME_MUTE,
	VOLUME_DOWN,
	VOLUME_UP,
	MEDIA_NEXT_TRACK,
	MEDIA_PREV_TRACK,
	MEDIA_STOP,
	MEDIA_PLAY_PAUSE,
	LAUNCH_MAIL,
	LAUNCH_MEDIA_SELECT,
	LAUNCH_APP1,
	LAUNCH_APP2,
	OEM_1, // ';:' for US
	OEM_PLUS, // '+' any country
	OEM_COMMA, // ',' any country
	OEM_MINUS, // '-' any country
	OEM_PERIOD,  // '.' any country
	OEM_2, // '/?' for US
	OEM_3, // '`~' for US
	OEM_4 = 0xDB, //  '[{' for US
	OEM_5, //  '\|' for US
	OEM_6, //  ']}' for US
	OEM_7,  //  ''"' for US
	OEM_8
};

extern "C" NET_API const char* GetKeyName(int);
extern "C" NET_API bool KeyWasPressed(int);
extern "C" NET_API bool KeyIsPressed(int);
extern "C" NET_API bool KeyIsPressed2(int);
END_NAMESPACE

BEGIN_NAMESPACE(Math)
template <typename TYPE>
TYPE GetRandNumber(TYPE min, TYPE max);
END_NAMESPACE

BEGIN_NAMESPACE(Random)
extern "C" NET_API void GetRandString(char** out, const size_t len);
END_NAMESPACE
END_NAMESPACE