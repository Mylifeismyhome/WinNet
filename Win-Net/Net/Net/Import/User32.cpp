#define MODULE_NAME CSTRING("User32")

#include "User32.h"

#ifndef NET_DISABLE_IMPORT_User32
#include <Net/Cryption/PointerCryption.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Import/MemoryModule.h>

namespace Net
{
	IMPORT_BEGIN_BODY(User32)
		IMPORT_HANDLE(HMEMORYMODULE, handle);

	IMPORT_DEFINE(MessageBoxA);
	IMPORT_DEFINE(MessageBoxW);

	IMPORT_INIT
		NET_FILEMANAGER fmanager(CSTRING("C:\\Windows\\System32\\User32.dll"), NET_FILE_READ);
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

	IMPORT_MPROCADDR(MessageBoxA);
	IMPORT_MPROCADDR(MessageBoxW);
	return true;
	IMPORT_END_INIT;

	IMPORT_UNLOAD
		if (!handle.valid())
			return;

	DELETE_IMPORT(MessageBoxA);
	DELETE_IMPORT(MessageBoxW);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END_UNLOAD;

	MAKE_IMPORT(int, MessageBoxA, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
		PASS_PARAMETERS(hWnd, lpText, lpCaption, uType);

	MAKE_IMPORT(int, MessageBoxW, HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
		PASS_PARAMETERS(hWnd, lpText, lpCaption, uType);
	IMPORT_END
}
#endif
