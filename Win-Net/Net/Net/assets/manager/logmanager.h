#pragma once
#include <Net/Net/Net.h>

typedef uint16_t WORD;

#define NET_ON_LOG_PARAMETERS_A int state, const char* buffer
#define NET_ON_LOG_PARAMETERS_W int state, const wchar_t* buffer

#ifdef UNICODE
#define NET_ON_LOG_PARAMETERS NET_ON_LOG_PARAMETERS_W
#define NET_ON_LOG NetSetLogCallbackW
#else
#define NET_ON_LOG_PARAMETERS NET_ON_LOG_PARAMETERS_A
#define NET_ON_LOG NetSetLogCallbackA
#endif

#include <Net/Net/NetString.h>
#include <Net/assets/assets.h>
#include <Net/assets/manager/filemanager.h>

typedef void (*OnLogA_t)(int state, const char* buffer);
typedef void (*OnLogW_t)(int state, const wchar_t* buffer);

void NetSetLogCallbackA(OnLogA_t);
void NetSetLogCallbackW(OnLogW_t);

NET_DSA_BEGIN

//// CONSOLE LOG ONLY
#define CLOG(...) \
Net::Console::Log(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__);

#define CLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__);

#define CLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__);

#define CLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__);

#ifdef DEBUG
#define CLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__);
#else
#define CLOG_DEBUG(...)
#endif

#define CLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__);

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
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__); \
}

#define LOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__); \
}

#define LOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__); \
}

#define LOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__); \
}

#ifdef DEBUG
#define LOG_DEBUG(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__); \
}
#else
#define LOG_DEBUG(...)
#endif

#define LOG_PEER(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__); \
}

////////////// LOG NO FUNCTION NAME
#define NLOG(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_ERROR(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_WARNING(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__); \
}

#define NLOG_SUCCESS(...) \
if(IsAreaInUse()) \
{ \
	Net::Manager::Log l; \
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
	Net::Manager::Log l; \
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
	Net::Manager::Log l; \
	l.doLog(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__); \
} \
else \
{ \
	Net::Console::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__); \
}
///////////////////////

/* LOG (FILENAME) */
#define BEGIN_LOG(fname) \
SetFname(CSTRING(fname)); \
SetAreaInUse(true);

#define END_LOG \
SetAreaInUse(false);

NET_API void SetFname(const char*);
NET_API char* GetFname();
NET_API void SetAreaInUse(bool);
NET_API bool IsAreaInUse();

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

NET_API tm TM_GetTime();
std::string GetLogStatePrefix(LogStates);
void Log(LogStates, const char*, const char*, ...);
void Log(LogStates, const char*, const wchar_t*, ...);
NET_API void ChangeStdOutputColor(int);

NET_API void SetPrintF(bool);
NET_API bool GetPrintFState();
NET_API WORD GetColorFromState(LogStates);
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Manager)
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
