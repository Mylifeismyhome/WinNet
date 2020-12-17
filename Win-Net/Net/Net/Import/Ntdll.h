#pragma once
#include <Net/Net/Net.h>
#include <winternl.h>

typedef NTSTATUS(NTAPI* DEF_lpNtCreateThreadEx)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, LPTHREAD_START_ROUTINE, LPVOID, ULONG, ULONG_PTR, SIZE_T, SIZE_T, LPVOID);

namespace Net
{
	namespace Ntdll
	{
		bool Initialize();
		void Uninitialize();

		NTSTATUS NtCreateThreadEx(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, LPTHREAD_START_ROUTINE, LPVOID, ULONG, ULONG_PTR, SIZE_T, SIZE_T, LPVOID);
	}
}