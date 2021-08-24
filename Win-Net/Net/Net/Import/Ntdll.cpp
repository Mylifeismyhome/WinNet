#define MODULE_NAME CSTRING("Ntdll")

#include "Ntdll.h"

#ifndef NET_DISABLE_IMPORT_NTDLL
#include <Net/Cryption/PointerCryption.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Import/MemoryModule.h>

namespace Net
{
	IMPORT_BEGIN(Ntdll)
		IMPORT_HANDLE(HMEMORYMODULE, handle);

	IMPORT_DEFINE(NtCreateThreadEx);
	IMPORT_DEFINE(NtQueryInformationProcess);

	IMPORT_INIT
		NET_FILEMANAGER fmanager(CSTRING("C:\\Windows\\System32\\ntdll.dll"), NET_FILE_READ);
	if (!fmanager.file_exists())
		return false;

	BYTE* module = nullptr;
	size_t size = NULL;
	if (!fmanager.read(module, size))
		return false;

	handle.Set(new HMEMORYMODULE(MemoryLoadLibrary(module, size)));
	if (!handle.valid())
	{
		LOG_DEBUG(CSTRING("[%s] - Unable to load ntdll"), MODULE_NAME);
		return false;
	}

	FREE(module);

	IMPORT_MPROCADDR(NtCreateThreadEx);
	IMPORT_MPROCADDR(NtQueryInformationProcess);
	return true;
	IMPORT_END_INIT;

	IMPORT_UNLOAD
		if (!handle.valid())
			return;

	DELETE_IMPORT(NtCreateThreadEx);
	DELETE_IMPORT(NtQueryInformationProcess);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END_UNLOAD;

	MAKE_IMPORT(NTSTATUS, NtCreateThreadEx, const PHANDLE hThread, const ACCESS_MASK DesiredAccess, const POBJECT_ATTRIBUTES ObjectAttributes, const HANDLE ProcessHandle, const LPTHREAD_START_ROUTINE lpStartAddress, const LPVOID lpParameter, const ULONG Flags, const ULONG_PTR StackZeroBits, const SIZE_T SizeOfStackCommit, const SIZE_T SizeOfStackReserve, const LPVOID lpBytesBuffer)
		PASS_PARAMETERS(hThread, DesiredAccess, ObjectAttributes, ProcessHandle, lpStartAddress, lpParameter, Flags, StackZeroBits, SizeOfStackCommit, SizeOfStackReserve, lpBytesBuffer);

	MAKE_IMPORT(NTSTATUS, NtQueryInformationProcess, HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
		PASS_PARAMETERS(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
	IMPORT_END
}
#endif
