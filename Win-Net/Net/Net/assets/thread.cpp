#include <Net/assets/thread.h>
#include <Net/Import/Kernel32.h>
#include <Net/Import/Ntdll.h>
#include <Net/assets/manager/logmanager.h>

#ifdef BUILD_LINUX
bool Net::Thread::Create(NET_THREAD_DWORD(*StartRoutine)(LPVOID), LPVOID const parameter)
{
	std::thread(StartRoutine, parameter).detach();
}
#else
bool Net::Thread::Create(NET_THREAD_DWORD(*StartRoutine)(LPVOID), LPVOID const parameter)
{
	auto handle = INVALID_HANDLE_VALUE;

	const auto status = (NTSTATUS)Net::Ntdll::NtCreateThreadEx(&handle, THREAD_ALL_ACCESS, nullptr, GetCurrentProcess(), nullptr, parameter, THREAD_TERMINATE, NULL, NULL, NULL, nullptr);
	if (status != NET_STATUS_SUCCESS)
	{
		LOG_DEBUG(CSTRING("[Thread] - NtCreateThreadEx failed with: %lu"), GetLastError());
		return false;
	}

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_ALL;
	auto ret = Net::Kernel32::GetThreadContext(handle, &ctx);
	if (!ret)
	{
		LOG_DEBUG(CSTRING("[Thread] - GetThreadContext failed"), GetLastError());
		return false;
	}

#ifdef _WIN64
	ctx.Rcx = (DWORD64)StartRoutine;
#else
	ctx.Eax = (DWORD)StartRoutine;
#endif

	ret = Net::Kernel32::SetThreadContext(handle, &ctx);
	if (!ret)
	{
		LOG_DEBUG(CSTRING("[Thread] - SetThreadContext failed"), GetLastError());
		return false;
	}

	if (Net::Kernel32::ResumeThread(handle) == (DWORD)-1)
	{
		LOG_DEBUG(CSTRING("[Thread] - ResumeThread failed with: %lu"), GetLastError());
		return false;
	}

	return true;
}
#endif
