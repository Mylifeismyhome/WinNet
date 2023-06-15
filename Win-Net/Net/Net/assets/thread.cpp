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

#include <Net/Net/Net.h>
#include <Net/assets/thread.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/Import/Ntdll.hpp>
#include <Net/assets/manager/logmanager.h>

#ifdef NET_THREAD_USE_STD
NET_THREAD_HANDLE Net::Thread::Create(NET_THREAD_FUNCTION StartRoutine, void* parameter = nullptr)
{
	pthread_t thread;
	const auto result = pthread_create(&thread, nullptr, StartRoutine, parameter);
	if (result != 0) 
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - Failed to create '%d'"), result);
		return 0;
	}
	return thread;
}

NET_THREAD_DWORD Net::Thread::WaitObject(NET_THREAD_HANDLE handle, NET_THREAD_DWORD t = INFINITE)
{
	void* returnValue;
	int result = pthread_join(handle, &returnValue);
	if (result != 0) 
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - Failed to join '%d'"), result);
		return 0;
	}
	return static_cast<NET_THREAD_DWORD>(returnValue);
}

void Net::Thread::Close(NET_THREAD_HANDLE handle)
{
	// In Linux, thread handles are not explicitly closed.
	// The resources associated with a thread are automatically released when the thread terminates.
	// Therefore, no explicit close is needed.
}
#else
NET_THREAD_HANDLE Net::Thread::Create(NET_THREAD_FUNCTION StartRoutine, LPVOID const parameter)
{
	auto handle = INVALID_HANDLE_VALUE;

	const auto status = (NTSTATUS)Ntdll::NtCreateThreadEx(&handle, THREAD_ALL_ACCESS, nullptr, GetCurrentProcess(), nullptr, parameter, THREAD_TERMINATE, NULL, NULL, NULL, nullptr);
	if (status != NET_STATUS_SUCCESS)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - NtCreateThreadEx failed with: %lu"), GetLastError());
		return nullptr;
	}

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_ALL;
	auto ret = Kernel32::GetThreadContext(handle, &ctx);
	if (!ret)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - GetThreadContext failed"), GetLastError());
		return nullptr;
	}

#ifdef _WIN64
	ctx.Rcx = (NET_THREAD_DWORD)StartRoutine;
#else
	ctx.Eax = (NET_THREAD_DWORD)StartRoutine;
#endif

	ret = Kernel32::SetThreadContext(handle, &ctx);
	if (!ret)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - SetThreadContext failed"), GetLastError());
		return nullptr;
	}

	if (Kernel32::ResumeThread(handle) == (DWORD)-1)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - ResumeThread failed with: %lu"), GetLastError());
		return nullptr;
	}

	return handle;
}

NET_THREAD_DWORD Net::Thread::WaitObject(NET_THREAD_HANDLE h, NET_THREAD_DWORD t)
{
	return Kernel32::WaitForSingleObject(h, static_cast<DWORD>(t));
}

void Net::Thread::Close(NET_THREAD_HANDLE h)
{
	Kernel32::CloseHandle(h);
}
#endif
