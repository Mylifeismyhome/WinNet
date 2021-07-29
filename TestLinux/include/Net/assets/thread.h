#pragma once
#define NET_no_init_all
#define NET_STATUS_SUCCESS    ((NTSTATUS)0x00000000L)
#define NET_ThreadQuerySetWin32StartAddress 9
#define NET_THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004

#include <Net/Net/Net.h>

#ifdef BUILD_LINUX
#include <thread>
#endif

// on linux we just use std::thread - on windows we use winapi to create threads
#ifdef BUILD_LINUX
typedef DWORD NET_THREAD_DWORD;
#define LPVOID void*
#define NET_THREAD(fnc) NET_THREAD_DWORD fnc(LPVOID parameter)
#else
#ifdef _WIN64
typedef DWORD64 NET_THREAD_DWORD;
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPI fnc(LPVOID parameter)
#else
typedef DWORD NET_THREAD_DWORD;
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPIV fnc(LPVOID parameter)
#endif
#endif

namespace Net
{
	namespace Thread
	{
#ifdef BUILD_LINUX
		bool Create(NET_THREAD_DWORD(*)(LPVOID), LPVOID parameter = nullptr);
#else
		bool Create(NET_THREAD_DWORD(*)(LPVOID), LPVOID parameter = nullptr);
#endif
	}
}
