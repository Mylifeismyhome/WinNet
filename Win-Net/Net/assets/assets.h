#pragma once
#include <Net/Net.h>
#include <assets/manager/dirmanager.h>
#include <Cryption/XOR.h>

NET_DSA_BEGIN

constexpr auto BASE_DATE = 1900;

#define TIME_LENGTH 6
#define TIME_LEN TIME_LENGTH
#define DATE_LENGTH 11
#define DATE_LEN DATE_LENGTH

// Messagebox
#define SHOW_MESSAGEBOX(msg, ...) Net::ShowMessageBox("", msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_ERROR(msg, ...) Net::ShowMessageBox(CSTRING("ERROR"), msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_SUCCESS(msg, ...) Net::ShowMessageBox(CSTRING("SUCCESS"), msg, __VA_ARGS__);
#define SHOW_MESSAGEBOX_DEBUG(msg, ...) Net::ShowMessageBox(CSTRING("DEBUG"), msg, __VA_ARGS__);

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

#define TIMER static double

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

NET_NAMESPACE_BEGIN(Net)
void ShowMessageBox(const char*, const char*, ...);
void ShowMessageBox(const wchar_t*, const wchar_t*, ...);

NET_NAMESPACE_BEGIN(Clock)
double GetClockTime();
double AddTime(double);
void GetTimeW(wchar_t*);
void GetTimeA(char*);
void GetDateW(wchar_t*);
void GetDateA(char*);
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Keyboard)
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
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Math)
template <typename TYPE>
TYPE GetRandNumber(TYPE min, TYPE max);
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Random)
extern "C" NET_API void GetRandString(char*& out, size_t len);
extern "C" NET_API void GetRandStringNew(BYTE * &out, size_t len);
extern "C" NET_API uintptr_t GetRandSeed();
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END