#pragma once
#include <Net/Net/Net.h>

#ifdef NET_DISABLE_IMPORT_NTDLL
#define Ntdll
#else
#include <Net/Import/Definition.hpp>
#include <winternl.h>

namespace Net
{
	IMPORT_BEGIN(Ntdll);
	IMPORT_DEFINE_FNC_WINAPI(NTSTATUS, NtCreateThreadEx, PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, HANDLE, LPTHREAD_START_ROUTINE, LPVOID, ULONG, ULONG_PTR, SIZE_T, SIZE_T, LPVOID);
	IMPORT_DEFINE_FNC_WINAPI(NTSTATUS, NtQueryInformationProcess, HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
	IMPORT_END
};
#endif
