#define MODULE_NAME CSTRING("Ntdll")

#include "Ws2_32.h"
#include <Net/Cryption/PointerCryption.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Import/MemoryModule.h>

namespace Net
{
	IMPORT_BEGIN(Ws2_32)
		IMPORT_HANDLE(HMEMORYMODULE, handle);

	IMPORT_DEFINE(send);
	IMPORT_DEFINE(recv);

	IMPORT_INIT
		NET_FILEMANAGER fmanager(CSTRING("C:\\Windows\\System32\\Ws2_32.dll"), NET_FILE_READ);
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

	IMPORT_MPROCADDR(send);
	IMPORT_MPROCADDR(recv);
	return true;
	IMPORT_END;

	IMPORT_UNLOAD
		if (!handle.valid())
			return;

	DELETE_IMPORT(send);
	DELETE_IMPORT(recv);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END;

	MAKE_IMPORT(int, send, SOCKET s, const char* buf, int len, int flags)
		PASS_PARAMETERS(s, buf, len, flags);

	MAKE_IMPORT(int, recv, SOCKET s, const char* buf, int len, int flags)
		PASS_PARAMETERS(s, buf, len, flags);
	IMPORT_END
}