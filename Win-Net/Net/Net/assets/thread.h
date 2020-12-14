#pragma once
#define no_init_all
#define STATUS_SUCCESS    ((NTSTATUS)0x00000000L)
#define ThreadQuerySetWin32StartAddress 9
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004

#include <Net/Net/Net.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Import/Kernel32.h>
#include <Net/Import/Ntdll.h>
#include <thread>

#ifdef _WIN64
typedef DWORD64 THREAD_DWORD;
#else
typedef DWORD THREAD_DWORD;
#endif

#define THREAD(fnc) THREAD_DWORD WINAPI fnc(LPVOID parameter)

namespace Net
{
	namespace Thread
	{
		bool Create(THREAD_DWORD(*)(LPVOID), LPVOID parameter = nullptr);
	}
}