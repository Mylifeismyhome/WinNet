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
