#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Security/NET_MemoryModule.h>

static bool Kernel32Initialized = false;

#define IMPORT(name, strname, type) name.Set(new type((type)MemoryGetProcAddress(*handle.get(), CSTRING(strname)))); \
	if(!##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[Kernel32] - Unable to resolve %s"), CSTRING(strname)); \
		return false; \
	}

#define DELETE_IMPORT(pointer) delete pointer.get(); pointer = nullptr;

#include "NET_Kernel32.h"

// Todo: add VMProtect to this library
#define PROTECT_ULTRA_BEGIN
#define PROTECT_END

namespace NET_Kernel32
{
#define DISABLE_MS_POINTERCRYPTION
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
	CPOINTER <DEF_EncodePointer> _EncodePointer;
	CPOINTER <DEF_DecodePointer> _DecodePointer;
#undef DISABLE_MS_POINTERCRYPTION
}

bool NET_Kernel32::Initialize()
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (Kernel32Initialized) return true;
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
		LOG_ERROR(CSTRING("[Kernel32] - LoadLibrary failed with error: %ld"), GetLastError());
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
	IMPORT(_EncodePointer, "EncodePointer", DEF_EncodePointer);
	IMPORT(_DecodePointer, "DecodePointer", DEF_DecodePointer);

	Kernel32Initialized = true;
	return true;
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

void NET_Kernel32::Uninitialize()
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
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
	DELETE_IMPORT(_EncodePointer);
	DELETE_IMPORT(_DecodePointer);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT(handle);

	Kernel32Initialized = false;
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

HRSRC NET_Kernel32::FindResourceA(const HMODULE handle, const LPCSTR lpName, const LPCSTR lpType)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_FindResourceA.get())(handle, lpName, lpType);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

HRSRC NET_Kernel32::FindResourceW(const HMODULE handle, const wchar_t* lpName, const wchar_t* lpType)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_FindResourceW.get())(handle, lpName, lpType);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

HGLOBAL NET_Kernel32::LoadResource(const HMODULE handle, HRSRC const hResInfo)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_LoadResource.get())(handle, hResInfo);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

DWORD NET_Kernel32::SizeofResource(const HMODULE handle, HRSRC const hResInfo)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return NULL;
	}

	return (*_SizeofResource.get())(handle, hResInfo);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

LPVOID NET_Kernel32::LockResource(const HGLOBAL hResData)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_LockResource.get())(hResData);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

HMODULE NET_Kernel32::LoadLibraryA(const LPCSTR lpLibFileName)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_LoadLibraryA.get())(lpLibFileName);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

HMODULE NET_Kernel32::LoadLibraryW(const wchar_t* lpLibFileName)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_LoadLibraryW.get())(lpLibFileName);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

FARPROC NET_Kernel32::GetProcAddress(const HMODULE hModule, const LPCSTR lpProcName)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return nullptr;
	}

	return (*_GetProcAddress.get())(hModule, lpProcName);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

BOOL NET_Kernel32::FreeLibrary(const HMODULE hModule)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return false;
	}

	return (*_FreeLibrary.get())(hModule);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

void NET_Kernel32::Sleep(const DWORD dwMilliseconds)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return;
	}

	return (*_Sleep.get())(dwMilliseconds);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

PVOID NET_Kernel32::EncodePointer(PVOID pointer)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return pointer;
	}

	return (*_EncodePointer.get())(pointer);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}

PVOID NET_Kernel32::DecodePointer(PVOID pointer)
{
#define DISABLE_MS_POINTERCRYPTION
	PROTECT_ULTRA_BEGIN;
	if (!Kernel32Initialized)
	{
		LOG_ERROR(CSTRING("[Kernel32] - Kernel32 Module is not been initialized yet."));
		return pointer;
	}

	return (*_DecodePointer.get())(pointer);
	PROTECT_END;
#undef DISABLE_MS_POINTERCRYPTION
}