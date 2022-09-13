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
#define NET_CNET_LOG(...)
#define NET_CNET_LOG_ERROR(...)
#define NET_CNET_LOG_WARNING(...)
#define NET_CNET_LOG_SUCCESS(...)
#else
#define NET_CNET_LOG(...) \
Net::Console::NET_LOG(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__);

#define NET_CNET_LOG_ERROR(...) \
Net::Console::NET_LOG(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__);

#define NET_CNET_LOG_WARNING(...) \
Net::Console::NET_LOG(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__);

#define NET_CNET_LOG_SUCCESS(...) \
Net::Console::NET_LOG(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NET_CNET_LOG_DEBUG(...)
#else
#define NET_CNET_LOG_DEBUG(...) \
Net::Console::NET_LOG(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__);
#endif
#else
#define NET_CNET_LOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NET_CNET_LOG_PEER(...)
#else
#define NET_CNET_LOG_PEER(...) \
Net::Console::NET_LOG(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__);
#endif

//////////////////// CLOG No function name
#ifdef NET_DISABLE_LOGMANAGER
#define NET_NCNET_LOG(...)
#define NET_NCNET_LOG_ERROR(...)
#define NET_NCNET_LOG_WARNING(...)
#define NET_NCNET_LOG_SUCCESS(...)
#else
#define NET_NCNET_LOG(...) \
Net::Console::NET_LOG(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__);

#define NET_NCNET_LOG_ERROR(...) \
Net::Console::NET_LOG(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__);

#define NET_NCNET_LOG_WARNING(...) \
Net::Console::NET_LOG(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__);

#define NET_NCNET_LOG_SUCCESS(...) \
Net::Console::NET_LOG(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NET_NCNET_LOG_DEBUG(...)
#else
#define NET_NCNET_LOG_DEBUG(...) \
Net::Console::NET_LOG(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__);
#endif
#else
#define NET_NCNET_LOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NET_NCNET_LOG_PEER(...)
#else
#define NET_NCNET_LOG_PEER(...) \
Net::Console::NET_LOG(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__);
#endif

//////////////////// LOG AND WRITE TO FILE
#ifdef NET_DISABLE_LOGMANAGER
#define NET_LOG(...)
#define NET_LOG_ERROR(...)
#define NET_LOG_WARNING(...)
#define NET_LOG_SUCCESS(...)
#else
#define NET_LOG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::normal, FUNCNAME, __VA_ARGS__);

#define NET_LOG_ERROR(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::error, FUNCNAME, __VA_ARGS__);

#define NET_LOG_WARNING(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::warning, FUNCNAME, __VA_ARGS__);

#define NET_LOG_SUCCESS(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::success, FUNCNAME, __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NET_LOG_DEBUG(...)
#else
#define NET_LOG_DEBUG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::debug, FUNCNAME, __VA_ARGS__);
#endif
#else
#define NET_LOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NET_LOG_PEER(...)
#else
#define NET_LOG_PEER(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::peer, FUNCNAME, __VA_ARGS__);
#endif

////////////// LOG NO FUNCTION NAME
#ifdef NET_DISABLE_LOGMANAGER
#define NET_NNET_LOG(...)
#define NET_NNET_LOG_ERROR(...)
#define NET_NNET_LOG_WARNING(...)
#define NET_NNET_LOG_SUCCESS(...)
#else
#define NET_NNET_LOG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::normal, CSTRING(""), __VA_ARGS__);

#define NET_NNET_LOG_ERROR(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::error, CSTRING(""), __VA_ARGS__);

#define NET_NNET_LOG_WARNING(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::warning, CSTRING(""), __VA_ARGS__);

#define NET_NNET_LOG_SUCCESS(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::success, CSTRING(""), __VA_ARGS__);
#endif

#ifdef DEBUG
#ifdef NET_DISABLE_LOGMANAGER
#define NET_NNET_LOG_DEBUG(...)
#else
#define NET_NNET_LOG_DEBUG(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::debug, CSTRING(""), __VA_ARGS__);
#endif
#else
#define NNET_LOG_DEBUG(...)
#endif

#ifdef NET_DISABLE_LOGMANAGER
#define NET_NNET_LOG_PEER(...)
#else
#define NET_NNET_LOG_PEER(...) \
	Net::Manager::Log::Log(Net::Console::LogStates::peer, CSTRING(""), __VA_ARGS__);
#endif
///////////////////////

/* LOG (FILENAME) */
#ifdef NET_DISABLE_LOGMANAGER
#define NET_BEGIN_NET_LOG(fname)
#define NET_END_LOG
#else
#define NET_BEGIN_NET_LOG(fname) \
	Net::Manager::Log::SetOutputName(fname);

#define NET_END_LOG \
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
NET_EXPORT_FUNCTION tm TM_GetTime();
#ifndef NET_DISABLE_LOGMANAGER
NET_EXPORT_FUNCTION std::string GetLogStatePrefix(LogStates);
void Log(LogStates, const char*, const char*, ...);
void Log(LogStates, const char*, const wchar_t*, ...);
NET_EXPORT_FUNCTION void ChangeStdOutputColor(int);

NET_EXPORT_FUNCTION void SetPrintF(bool);
NET_EXPORT_FUNCTION bool GetPrintFState();
NET_EXPORT_FUNCTION WORD GetColorFromState(LogStates);
#endif
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(Manager)
NET_NAMESPACE_BEGIN(Log)
#ifndef NET_DISABLE_LOGMANAGER
NET_EXPORT_FUNCTION void SetOutputName(const char*);
NET_EXPORT_FUNCTION void SetLogCallbackA(OnLogA_t);
NET_EXPORT_FUNCTION void SetLogCallbackW(OnLogW_t);
NET_EXPORT_FUNCTION void Log(Console::LogStates, const char*, const char*, ...);
void Log(Console::LogStates, const char*, const wchar_t*, ...);
#endif
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
