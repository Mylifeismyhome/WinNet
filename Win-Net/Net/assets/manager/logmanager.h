#pragma once
#include <Net/Net.h>
#include "assets/assets.h"
#include "filemanager.h"
#include "dirmanager.h"

NET_DSA_BEGIN

// Color codes
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGRAY 7
#define DARKGRAY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15

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
Net::Console::Log(Net::Console::LogStates::normal, "", __VA_ARGS__);

#define NCLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, "", __VA_ARGS__);

#define NCLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, "", __VA_ARGS__);

#define NCLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, "", __VA_ARGS__);

#ifdef DEBUG
#define NCLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, "", __VA_ARGS__);
#else
#define NCLOG_DEBUG(...)
#endif

#define NCLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, "", __VA_ARGS__);

//////////////////// LOG AND WRITE TO FILE
#define LOG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::normal, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, __func__, __VA_ARGS__); \
}

#define LOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::error, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, __func__, __VA_ARGS__); \
}

#define LOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::warning, __func__, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, __func__, __VA_ARGS__); \
}

#define LOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::success, __func__, __VA_ARGS__); \
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
	l.doLog(Net::manager::Log::LogManagerStates::debug, __func__, __VA_ARGS__); \
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
	l.doLog(Net::manager::Log::LogManagerStates::peer, __func__, __VA_ARGS__); \
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
	l.doLog(Net::manager::Log::LogManagerStates::normal, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, "", __VA_ARGS__); \
}

#define NLOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::error, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, "", __VA_ARGS__); \
}

#define NLOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::warning, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, "", __VA_ARGS__); \
}

#define NLOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::success, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::success, "", __VA_ARGS__); \
}

#ifdef DEBUG
#define NLOG_DEBUG(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::debug, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::debug, "", __VA_ARGS__); \
}
#else
#define NLOG_DEBUG(...)
#endif

#define NLOG_PEER(...) \
if(IsAreaInUse()) \
{ \
	Net::manager::Log l; \
	l.doLog(Net::manager::Log::LogManagerStates::peer, "", __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::peer, "", __VA_ARGS__); \
}
///////////////////////

/* LOG AREAS (FILENAME) */
#define BEGIN_AREA_LOG(fname) \
SetFname(fname); \
SetAreaInUse(true);

#define END_AREA_LOG \
SetAreaInUse(false);

static char fname[MAX_PATH];
static bool AreaInUse = false;
extern "C" NET_API void SetFname(const char*);
extern "C" NET_API char* GetFname();
extern "C" NET_API void SetAreaInUse(bool);
extern "C" NET_API bool IsAreaInUse();

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
NET_CLASS_BEGIN(Log)
NET_FILEMANAGER* file;
bool WriteToFile(const char*) const;
bool WriteToFile(const wchar_t*) const;
NET_CLASS_PUBLIC

NET_CLASS_CONSTRUCTUR(Log)
NET_CLASS_DESTRUCTUR(Log)

// have them sync with LogStates from Assets.h
enum class LogManagerStates
{
	normal = 0,
	debug,
	warning,
	error,
	success,
	peer
};

void doLog(Net::manager::Log::LogManagerStates, const char*, const char*, ...) const;
void doLog(Net::manager::Log::LogManagerStates, const char*, const wchar_t*, ...) const;
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END