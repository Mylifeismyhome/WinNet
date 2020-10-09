#pragma once
#include <Net/Net.h>
#include <Net/NetString.h>
#include <assets/assets.h>
#include <assets/manager/filemanager.h>

NET_DSA_BEGIN

// Color codes
constexpr auto BLACK = 0;
constexpr auto BLUE = 1;
constexpr auto GREEN = 2;
constexpr auto CYAN = 3;
constexpr auto RED = 4;
constexpr auto MAGENTA = 5;
constexpr auto BROWN = 6;
constexpr auto LIGHTGRAY = 7;
constexpr auto DARKGRAY = 8;
constexpr auto LIGHTBLUE = 9;
constexpr auto LIGHTGREEN = 10;
constexpr auto LIGHTCYAN = 11;
constexpr auto LIGHTRED = 12;
constexpr auto LIGHTMAGENTA = 13;
constexpr auto YELLOW = 14;
constexpr auto WHITE = 15;

//// CONSOLE LOG ONLY
#define CLOG(...) \
Net::Console::Log(Net::Console::LogStates::normal, __func__, __VA_ARGS__);

#define CLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, __func__, __VA_ARGS__);

#define CLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, __func__, __VA_ARGS__);

#define CLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, __func__, __VA_ARGS__);

#ifdef DEBUG
#define CLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, __func__, __VA_ARGS__);
#else
#define CLOG_DEBUG(...)
#endif

#define CLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, __func__, __VA_ARGS__);

//////////////////// CLOG No function name
#define NCLOG(...) \
Net::Console::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__);

#define NCLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__);

#define NCLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__);

#define NCLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__);

#ifdef DEBUG
#define NCLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__);
#else
#define NCLOG_DEBUG(...)
#endif

#define NCLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__);

//////////////////// LOG AND WRITE TO FILE
#define LOG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::normal, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, __func__, __VA_ARGS__); \
}

#define LOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::error, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, __func__, __VA_ARGS__); \
}

#define LOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::warning, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, __func__, __VA_ARGS__); \
}

#define LOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::success, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::success, __func__, __VA_ARGS__); \
}

#ifdef DEBUG
#define LOG_DEBUG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::debug, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::debug, __func__, __VA_ARGS__); \
}
#else
#define LOG_DEBUG(...)
#endif

#define LOG_PEER(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::peer, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::peer, __func__, __VA_ARGS__); \
}

////////////// LOG NO FUNCTION NAME
#define NLOG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__); \
}

#ifdef DEBUG
#define NLOG_DEBUG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__); \
}
#else
#define NLOG_DEBUG(...)
#endif

#define NLOG_PEER(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__); \
}
///////////////////////

/* LOG (FILENAME) */
#define BEGIN_LOG(fname) \
SetFname(fname); \
SetAreaInUse(true);

#define END_LOG \
SetAreaInUse(false);

static char fname[MAX_PATH];
static bool AreaInUse = false;
extern "C" NET_API void SetFname(const char*);
extern "C" NET_API char* GetFname();
extern "C" NET_API void SetAreaInUse(bool);
extern "C" NET_API bool IsAreaInUse();

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Console)
enum class LogStates
{
	normal = 0,
	debug,
	warning,
	error,
	success,
	peer
};

extern "C" NET_API tm TM_GetTime();
std::string GetLogStatePrefix(LogStates);
void Log(LogStates, const char*, const char*, ...);
void Log(LogStates, const char*, const wchar_t*, ...);
extern "C" NET_API void ChangeStdOutputColor(int);

extern "C" NET_API void SetPrintF(bool);
extern "C" NET_API bool GetPrintFState();
extern "C" NET_API WORD GetColorFromState(LogStates);
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(manager)
NET_CLASS_BEGIN(Log)
NET_FILEMANAGER* file;
bool WriteToFile(const char*) const;
bool WriteToFile(const wchar_t*) const;
NET_CLASS_PUBLIC

NET_CLASS_CONSTRUCTUR(Log)
NET_CLASS_DESTRUCTUR(Log)
void doLog(Console::LogStates, const char*, const char*, ...) const;
void doLog(Console::LogStates, const char*, const wchar_t*, ...) const;
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END