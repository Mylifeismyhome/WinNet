#pragma once
#include <Net/Net/Net.h>

typedef uint16_t WORD;

#define NET_ON_LOG_PARAMETERS_A int state, const char* buffer
#define NET_ON_LOG_PARAMETERS_W int state, const wchar_t* buffer

#ifdef UNICODE
#define NET_ON_LOG_PARAMETERS NET_ON_LOG_PARAMETERS_W
#define NET_ON_LOG Net::Manager::Log::SetLogCallbackW
#else
#define NET_ON_LOG_PARAMETERS NET_ON_LOG_PARAMETERS_A
#define NET_ON_LOG Net::Manager::Log::SetLogCallbackA
#endif

#include <Net/Net/NetString.h>

typedef void (*OnLogA_t)(int state, const char* buffer);
typedef void (*OnLogW_t)(int state, const wchar_t* buffer);

NET_DSA_BEGIN

//// CONSOLE LOG ONLY
#ifdef NET_DISABLE_LOGMANAGER
#define CLOG(...)
#define CLOG_ERROR(...)
#define CLOG_WARNING(...)
#define CLOG_SUCCESS(...)
#else
#define CLOG(...) \
Net::Console::Log(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__);

#define CLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__);

#define CLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__);

#define CLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define CLOG_DEBUG(...)
#else
#define CLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__);
#endif
#else
#define CLOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define CLOG_PEER(...)
#else
#define CLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__);
#endif

//////////////////// CLOG No function name
#ifdef NET_DISABLE_LOGMANAGER
#define NCLOG(...)
#define NCLOG_ERROR(...)
#define NCLOG_WARNING(...)
#define NCLOG_SUCCESS(...)
#else
#define NCLOG(...) \
Net::Console::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__);

#define NCLOG_ERROR(...) \
Net::Console::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__);

#define NCLOG_WARNING(...) \
Net::Console::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__);

#define NCLOG_SUCCESS(...) \
Net::Console::Log(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NCLOG_DEBUG(...)
#else
#define NCLOG_DEBUG(...) \
Net::Console::Log(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__);
#endif
#else
#define NCLOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NCLOG_PEER(...)
#else
#define NCLOG_PEER(...) \
Net::Console::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__);
#endif

//////////////////// LOG AND WRITE TO FILE
#ifdef NET_DISABLE_LOGMANAGER
#define LOG(...)
#define LOG_ERROR(...)
#define LOG_WARNING(...)
#define LOG_SUCCESS(...)
#else
#define LOG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__);

#define LOG_ERROR(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__);

#define LOG_WARNING(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__);

#define LOG_SUCCESS(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define LOG_DEBUG(...)
#else
#define LOG_DEBUG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__);
#endif
#else
#define LOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define LOG_PEER(...)
#else
#define LOG_PEER(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__);
#endif

////////////// LOG NO FUNCTION NAME
#ifdef NET_DISABLE_LOGMANAGER
#define NLOG(...)
#define NLOG_ERROR(...)
#define NLOG_WARNING(...)
#define NLOG_SUCCESS(...)
#else
#define NLOG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__);

#define NLOG_ERROR(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__);

#define NLOG_WARNING(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__);

#define NLOG_SUCCESS(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NLOG_DEBUG(...)
#else
#define NLOG_DEBUG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__);
#endif
#else
#define NLOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NLOG_PEER(...)
#else
#define NLOG_PEER(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__);
#endif
///////////////////////

/* LOG (FILENAME) */
#ifdef NET_DISABLE_LOGMANAGER
#define BEGIN_LOG(fname)
#define END_LOG
#else
#define BEGIN_LOG(fname) \
	Net::Manager::Log::SetOutputName(fname);

#define END_LOG \
	Net::Manager::Log::SetOutputName(CSTRING(""));
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Console)
#ifndef NET_DISABLE_LOGMANAGER
enum class LogStates
{
	normal = 0,
	debug,
	warning,
	error,
	success,
	peer
};
#endif
tm TM_GetTime();
#ifndef NET_DISABLE_LOGMANAGER
std::string GetLogStatePrefix(LogStates);
void Log(LogStates, const char*, const char*, ...);
void Log(LogStates, const char*, const wchar_t*, ...);
void ChangeStdOutputColor(int);

void SetPrintF(bool);
bool GetPrintFState();
WORD GetColorFromState(LogStates);
#endif
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Manager)
NET_NAMESPACE_BEGIN(Log)
#ifndef NET_DISABLE_LOGMANAGER
void SetOutputName(const char*);
void SetLogCallbackA(OnLogA_t);
void SetLogCallbackW(OnLogW_t);
void Log(Console::LogStates, const char*, const char*, ...);
void Log(Console::LogStates, const char*, const wchar_t*, ...);
#endif
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
