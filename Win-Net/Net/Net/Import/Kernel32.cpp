#define MODULE_NAME CSTRING("Kernel32")

#define IMPORT(name, strname, type) name.Set(new type((type)MemoryGetProcAddress(*handle.get(), CSTRING(strname)))); \
	if(!##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[%s] - Unable to resolve %s"), MODULE_NAME, CSTRING(strname)); \
		return false; \
	}

#define DELETE_IMPORT(pointer) delete pointer.get(); pointer = nullptr;

#include <Net/Import/Kernel32.h>

namespace Net
{
	namespace Kernel32
	{
		CPOINTER<HMEMORYMODULE> handle;

		CPOINTER <DEF_FindResourceA> _FindResourceA;
		CPOINTER <DEF_FindResourceW> _FindResourceW;
		CPOINTER <DEF_LoadResource> _LoadResource;
		CPOINTER <DEF_SizeofResource> _SizeofResource;
		CPOINTER <DEF_LockResource> _LockResource;
		CPOINTER <DEF_LoadLibraryA> _LoadLibraryA;
		CPOINTER <DEF_LoadLibraryW> _LoadLibraryW;
		CPOINTER <DEF_GetProcAddress> _GetProcAddress;
		CPOINTER <DEF_FreeLibrary> _FreeLibrary;
		CPOINTER <DEF_Sleep> _Sleep;
	}
}

bool Net::Kernel32::Initialize()
{
	NET_FILEMANAGER fmanager(CSTRING("C:\\Windows\\System32\\kernel32.dll"), NET_FILE_READ);
	if (!fmanager.file_exists())
		return false;

	BYTE* module = nullptr;
	size_t size = NULL;
	if (!fmanager.read(module, size))
		return false;
	
	handle.Set(new HMEMORYMODULE(MemoryLoadLibrary(module, size)));
	if (!handle.valid())
	{
		LOG_ERROR(CSTRING("[%s] - LoadLibrary failed with error: %ld"), MODULE_NAME, GetLastError());
		return false;
	}

	FREE(module);

	IMPORT(_FindResourceA, "FindResourceA", DEF_FindResourceA);
	IMPORT(_FindResourceW, "FindResourceW", DEF_FindResourceW);
	IMPORT(_LoadResource, "LoadResource", DEF_LoadResource);
	IMPORT(_SizeofResource, "SizeofResource", DEF_SizeofResource);
	IMPORT(_LockResource, "LockResource", DEF_LockResource);
	IMPORT(_LoadLibraryA, "LoadLibraryA", DEF_LoadLibraryA);
	IMPORT(_LoadLibraryW, "LoadLibraryW", DEF_LoadLibraryW);
	IMPORT(_GetProcAddress, "GetProcAddress", DEF_GetProcAddress);
	IMPORT(_FreeLibrary, "FreeLibrary", DEF_FreeLibrary);
	IMPORT(_Sleep, "Sleep", DEF_Sleep);

	return true;
}

void Net::Kernel32::Uninitialize()
{
	if (!handle.valid())
		return;

	DELETE_IMPORT(_FindResourceA);
	DELETE_IMPORT(_FindResourceW);
	DELETE_IMPORT(_LoadResource);
	DELETE_IMPORT(_SizeofResource);
	DELETE_IMPORT(_LockResource);
	DELETE_IMPORT(_LoadLibraryA);
	DELETE_IMPORT(_LoadLibraryW);
	DELETE_IMPORT(_GetProcAddress);
	DELETE_IMPORT(_FreeLibrary);
	DELETE_IMPORT(_Sleep);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT(handle);
}

HRSRC Net::Kernel32::FindResourceA(const HMODULE handle, const LPCSTR lpName, const LPCSTR lpType)
{
	return (*_FindResourceA.get())(handle, lpName, lpType);
}

HRSRC Net::Kernel32::FindResourceW(const HMODULE handle, const wchar_t* lpName, const wchar_t* lpType)
{
	return (*_FindResourceW.get())(handle, lpName, lpType);
}

HGLOBAL Net::Kernel32::LoadResource(const HMODULE handle, HRSRC const hResInfo)
{
	return (*_LoadResource.get())(handle, hResInfo);
}

DWORD Net::Kernel32::SizeofResource(const HMODULE handle, HRSRC const hResInfo)
{
	return (*_SizeofResource.get())(handle, hResInfo);
}

LPVOID Net::Kernel32::LockResource(const HGLOBAL hResData)
{
	return (*_LockResource.get())(hResData);
}

HMODULE Net::Kernel32::LoadLibraryA(const LPCSTR lpLibFileName)
{
	return (*_LoadLibraryA.get())(lpLibFileName);
}

HMODULE Net::Kernel32::LoadLibraryW(const wchar_t* lpLibFileName)
{
	return (*_LoadLibraryW.get())(lpLibFileName);
}

FARPROC Net::Kernel32::GetProcAddress(const HMODULE hModule, const LPCSTR lpProcName)
{
	return (*_GetProcAddress.get())(hModule, lpProcName);
}

BOOL Net::Kernel32::FreeLibrary(const HMODULE hModule)
{
	return (*_FreeLibrary.get())(hModule);
}

void Net::Kernel32::Sleep(const DWORD dwMilliseconds)
{
	return (*_Sleep.get())(dwMilliseconds);
}