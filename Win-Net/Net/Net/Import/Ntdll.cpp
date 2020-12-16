#define NET_MODULE_NAME CSTRING("Ntdll")

#define NET_IMPORT(name, strname, type) 	name.Set(new type((type)MemoryGetProcAddress(*handle.get(), CSTRING(strname)))); \
	if(!##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[%s] - Unable to resolve %s"), NET_MODULE_NAME, CSTRING(strname)); \
		return false; \
	}

#define NET_DELETE_IMPORT(pointer) delete pointer.get(); pointer = nullptr;

#include <Net/Import/Ntdll.h>
#include <Net/Import/MemoryModule.h>

namespace Net
{
	namespace Ntdll
	{
		CPOINTER<HMEMORYMODULE> handle;

		CPOINTER<DEF_lpNtCreateThreadEx> _NtCreateThreadEx;
	}
}

bool Net::Ntdll::Initialize()
{
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

	NET_IMPORT(_NtCreateThreadEx, "NtCreateThreadEx", DEF_lpNtCreateThreadEx);
	return true;
}

void Net::Ntdll::Uninitialize()
{
	if (!handle.valid())
		return;

	NET_DELETE_IMPORT(_NtCreateThreadEx);

	MemoryFreeLibrary(*handle.get());
	NET_DELETE_IMPORT(handle);
}

NTSTATUS Net::Ntdll::NtCreateThreadEx(const PHANDLE hThread, const ACCESS_MASK DesiredAccess, const POBJECT_ATTRIBUTES ObjectAttributes, const HANDLE ProcessHandle, const LPTHREAD_START_ROUTINE lpStartAddress, const LPVOID lpParameter, const ULONG Flags, const ULONG_PTR StackZeroBits, const SIZE_T SizeOfStackCommit, const SIZE_T SizeOfStackReserve, const LPVOID lpBytesBuffer)
{
	return (*_NtCreateThreadEx.get())(hThread, DesiredAccess, ObjectAttributes, ProcessHandle, lpStartAddress, lpParameter, Flags, StackZeroBits, SizeOfStackCommit, SizeOfStackReserve, lpBytesBuffer);
}