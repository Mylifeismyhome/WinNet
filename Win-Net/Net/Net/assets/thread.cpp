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

#include <Net/assets/thread.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/Import/Ntdll.hpp>
#include <Net/assets/manager/logmanager.h>

#ifdef BUILD_LINUX
bool Net::Thread::Create(NET_THREAD_DWORD(*StartRoutine)(LPVOID), LPVOID const parameter)
{
	std::thread(StartRoutine, parameter).detach();
	return true;
}
#else
bool Net::Thread::Create(NET_THREAD_DWORD(*StartRoutine)(LPVOID), LPVOID const parameter)
{
#ifdef NET_DISABLE_IMPORT_NTDLL
	std::thread(StartRoutine, parameter).detach();
	return true;
#else
	auto handle = INVALID_HANDLE_VALUE;

	const auto status = (NTSTATUS)Ntdll::NtCreateThreadEx(&handle, THREAD_ALL_ACCESS, nullptr, GetCurrentProcess(), nullptr, parameter, THREAD_TERMINATE, NULL, NULL, NULL, nullptr);
	if (status != NET_STATUS_SUCCESS)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - NtCreateThreadEx failed with: %lu"), GetLastError());
		return false;
	}

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_ALL;
	auto ret =	Kernel32::GetThreadContext(handle, &ctx);
	if (!ret)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - GetThreadContext failed"), GetLastError());
		return false;
	}

#ifdef _WIN64
	ctx.Rcx = (DWORD64)StartRoutine;
#else
	ctx.Eax = (DWORD)StartRoutine;
#endif

	ret = Kernel32::SetThreadContext(handle, &ctx);
	if (!ret)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - SetThreadContext failed"), GetLastError());
		return false;
	}

	if (Kernel32::ResumeThread(handle) == (DWORD)-1)
	{
		NET_LOG_DEBUG(CSTRING("[Thread] - ResumeThread failed with: %lu"), GetLastError());
		return false;
	}

	return true;
#endif
}
#endif
