#pragma once
#define NET_no_init_all
#define NET_STATUS_SUCCESS    ((NTSTATUS)0x00000000L)
#define NET_ThreadQuerySetWin32StartAddress 9
#define NET_THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004

#include <Net/Net/Net.h>

#ifdef _WIN64
typedef DWORD64 NET_THREAD_DWORD;
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPI fnc(LPVOID parameter)
#else
typedef DWORD NET_THREAD_DWORD;
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPIV fnc(LPVOID parameter)
#endif

namespace Net
{
	namespace Thread
	{
		bool Create(NET_THREAD_DWORD(*)(LPVOID), LPVOID parameter = nullptr);
	}
}