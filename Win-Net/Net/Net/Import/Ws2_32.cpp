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
	IMPORT_DEFINE(closesocket);
	IMPORT_DEFINE(WSACleanup);
	IMPORT_DEFINE(freeaddrinfo);
	IMPORT_DEFINE(WSAGetLastError);
	IMPORT_DEFINE(WSAStartup);
	IMPORT_DEFINE(getaddrinfo);
	IMPORT_DEFINE(socket);
	IMPORT_DEFINE(ioctlsocket);
	IMPORT_DEFINE(bind);
	IMPORT_DEFINE(listen);
	IMPORT_DEFINE(setsockopt);

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
	IMPORT_MPROCADDR(closesocket);
	IMPORT_MPROCADDR(WSACleanup);
	IMPORT_MPROCADDR(freeaddrinfo);
	IMPORT_MPROCADDR(WSAGetLastError);
	IMPORT_MPROCADDR(WSAStartup);
	IMPORT_MPROCADDR(getaddrinfo);
	IMPORT_MPROCADDR(socket);
	IMPORT_MPROCADDR(ioctlsocket);
	IMPORT_MPROCADDR(bind);
	IMPORT_MPROCADDR(listen);
	IMPORT_MPROCADDR(setsockopt);
	return true;
	IMPORT_END;

	IMPORT_UNLOAD
		if (!handle.valid())
			return;

	DELETE_IMPORT(send);
	DELETE_IMPORT(recv);
	DELETE_IMPORT(closesocket);
	DELETE_IMPORT(WSACleanup);
	DELETE_IMPORT(freeaddrinfo);
	DELETE_IMPORT(WSAGetLastError);
	DELETE_IMPORT(WSAStartup);
	DELETE_IMPORT(getaddrinfo);
	DELETE_IMPORT(socket);
	DELETE_IMPORT(ioctlsocket);
	DELETE_IMPORT(bind);
	DELETE_IMPORT(listen);
	DELETE_IMPORT(setsockopt);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END;

	MAKE_IMPORT(int, send, SOCKET s, const char* buf, int len, int flags)
		PASS_PARAMETERS(s, buf, len, flags);

	MAKE_IMPORT(int, recv, SOCKET s, const char* buf, int len, int flags)
		PASS_PARAMETERS(s, buf, len, flags);

	MAKE_IMPORT(int, closesocket, SOCKET s)
		PASS_PARAMETERS(s);

	MAKE_IMPORT(int, WSACleanup)
		NO_PARAMETERS;

	MAKE_IMPORT(VOID, freeaddrinfo, PADDRINFOA pAddrInfo)
		PASS_PARAMETERS(pAddrInfo);

	MAKE_IMPORT(int, WSAGetLastError)
		NO_PARAMETERS;

	MAKE_IMPORT(int, WSAStartup, WORD wVersionRequired, LPWSADATA lpWSAData)
		PASS_PARAMETERS(wVersionRequired, lpWSAData);

	MAKE_IMPORT(int, getaddrinfo, PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult)
		PASS_PARAMETERS(pNodeName, pServiceName, pHints, ppResult);

	MAKE_IMPORT(SOCKET, socket, int af, int type, int protocol)
		PASS_PARAMETERS(af, type, protocol);

	MAKE_IMPORT(int, ioctlsocket, SOCKET s, long cmd, u_long* argp)
		PASS_PARAMETERS(s, cmd, argp);

	MAKE_IMPORT(int, bind, SOCKET s, const struct sockaddr FAR* addr, int namelen)
		PASS_PARAMETERS(s, addr, namelen);

	MAKE_IMPORT(int, listen, SOCKET s, int backlog)
		PASS_PARAMETERS(s, backlog);

	MAKE_IMPORT(int, setsockopt, SOCKET s, int level, int optname, const char* optval, int optlen)
		PASS_PARAMETERS(s, level, optname, optval, optlen);
	IMPORT_END
}