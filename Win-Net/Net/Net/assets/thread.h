/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#include <Net/Net/Net.h>

#ifdef BUILD_LINUX
#include <pthread.h>
#else
#define NET_STATUS_SUCCESS    ((NTSTATUS)0x00000000L)
#define NET_ThreadQuerySetWin32StartAddress 9
#define NET_THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004
#endif

#define INFINITE 0xFFFFFFFF

#ifdef BUILD_LINUX
typedef unsigned int NET_THREAD_DWORD;
typedef void*(*NET_THREAD_FUNCTION)(void*);
#define NET_THREAD(fnc) void* fnc(void* parameter)
#define NET_THREAD_HANDLE pthread_t
#else
#ifdef _WIN64
typedef DWORD64 NET_THREAD_DWORD;
typedef NET_THREAD_DWORD(*NET_THREAD_FUNCTION)(LPVOID);
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPI fnc(LPVOID parameter)
#define NET_THREAD_HANDLE HANDLE
#else
typedef DWORD NET_THREAD_DWORD;
typedef NET_THREAD_DWORD(*NET_THREAD_FUNCTION)(LPVOID);
#define NET_THREAD(fnc) NET_THREAD_DWORD WINAPIV fnc(LPVOID parameter)
#define NET_THREAD_HANDLE HANDLE
#endif
#endif

namespace Net
{
	namespace Thread
	{
#ifdef BUILD_LINUX
		NET_THREAD_HANDLE Create(NET_THREAD_FUNCTION StartRoutine, void* parameter = nullptr);
		NET_THREAD_DWORD WaitObject(NET_THREAD_HANDLE h, NET_THREAD_DWORD t = INFINITE);
		void Close(NET_THREAD_HANDLE h);
#else
		NET_THREAD_HANDLE Create(NET_THREAD_FUNCTION StartRoutine, LPVOID parameter = nullptr);
		NET_THREAD_DWORD WaitObject(NET_THREAD_HANDLE h, NET_THREAD_DWORD t = INFINITE);
		void Close(NET_THREAD_HANDLE h);
#endif
	}
}
