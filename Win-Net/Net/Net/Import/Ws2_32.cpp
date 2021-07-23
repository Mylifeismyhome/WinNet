#define MODULE_NAME CSTRING("Ws2_32")

#include "Ws2_32.h"

#ifndef NET_DISABLE_IMPORT_WS2_32
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
	IMPORT_DEFINE(connect);
	IMPORT_DEFINE(accept);
	IMPORT_DEFINE(inet_pton);
	IMPORT_DEFINE(htons);
	IMPORT_DEFINE(htonl);
	IMPORT_DEFINE(WSARecvFrom);
	IMPORT_DEFINE(WSAIoctl);
	IMPORT_DEFINE(inet_ntop);
	IMPORT_DEFINE(sendto);
	IMPORT_DEFINE(WSAGetOverlappedResult);
	IMPORT_DEFINE(WSAResetEvent);
	IMPORT_DEFINE(getnameinfo);
	IMPORT_DEFINE(ntohl);
	IMPORT_DEFINE(recvfrom);
	IMPORT_DEFINE(inet_addr);
	IMPORT_DEFINE(ntohs);
	IMPORT_DEFINE(ntohll);

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
		LOG_DEBUG(CSTRING("[%s] - Unable to load Ws2_32"), MODULE_NAME);
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
	IMPORT_MPROCADDR(connect);
	IMPORT_MPROCADDR(accept);
	IMPORT_MPROCADDR(inet_pton);
	IMPORT_MPROCADDR(htons);
	IMPORT_MPROCADDR(htonl);
	IMPORT_MPROCADDR(WSARecvFrom);
	IMPORT_MPROCADDR(WSAIoctl);
	IMPORT_MPROCADDR(inet_ntop);
	IMPORT_MPROCADDR(sendto);
	IMPORT_MPROCADDR(WSAGetOverlappedResult);
	IMPORT_MPROCADDR(WSAResetEvent);
	IMPORT_MPROCADDR(getnameinfo);
	IMPORT_MPROCADDR(ntohl);
	IMPORT_MPROCADDR(recvfrom);
	IMPORT_MPROCADDR(inet_addr);
	IMPORT_MPROCADDR(ntohs);
	IMPORT_MPROCADDR(ntohll);

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
	DELETE_IMPORT(connect);
	DELETE_IMPORT(accept);
	DELETE_IMPORT(inet_pton);
	DELETE_IMPORT(htons);
	DELETE_IMPORT(htonl);
	DELETE_IMPORT(WSARecvFrom);
	DELETE_IMPORT(WSAIoctl);
	DELETE_IMPORT(inet_ntop);
	DELETE_IMPORT(sendto);
	DELETE_IMPORT(WSAGetOverlappedResult);
	DELETE_IMPORT(WSAResetEvent);
	DELETE_IMPORT(getnameinfo);
	DELETE_IMPORT(ntohl);
	DELETE_IMPORT(recvfrom);
	DELETE_IMPORT(inet_addr);
	DELETE_IMPORT(ntohs);
	DELETE_IMPORT(ntohll);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END;

	MAKE_IMPORT(int, send, SOCKET s, const char* buf, int len, int flags)
		PASS_PARAMETERS(s, buf, len, flags);

	MAKE_IMPORT(int, recv, SOCKET s, char* buf, int len, int flags)
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

	MAKE_IMPORT(int, connect, SOCKET s, const sockaddr* name, int namelen)
		PASS_PARAMETERS(s, name, namelen);

	MAKE_IMPORT(SOCKET, accept, SOCKET s, sockaddr* addr, int* addrlen)
		PASS_PARAMETERS(s, addr, addrlen);

	MAKE_IMPORT(INT, inet_pton, INT Family, PCSTR pszAddrString, PVOID pAddrBuf)
		PASS_PARAMETERS(Family, pszAddrString, pAddrBuf);

	MAKE_IMPORT(u_short, htons, u_short hostshort)
		PASS_PARAMETERS(hostshort);

	MAKE_IMPORT(u_short, htonl, u_short hostlong)
		PASS_PARAMETERS(hostlong);

	MAKE_IMPORT(int, WSARecvFrom, SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
		PASS_PARAMETERS(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);

	MAKE_IMPORT(int, WSAIoctl, SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
		PASS_PARAMETERS(s, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);

	MAKE_IMPORT(PCSTR, inet_ntop, INT Family, const VOID* pAddr, PSTR pStringBuf, size_t StringBufSize)
		PASS_PARAMETERS(Family, pAddr, pStringBuf, StringBufSize);

	MAKE_IMPORT(int, sendto, SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen)
		PASS_PARAMETERS(s, buf, len, flags, to, tolen);

	MAKE_IMPORT(BOOL, WSAGetOverlappedResult, SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags)
		PASS_PARAMETERS(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags);

	MAKE_IMPORT(BOOL, WSAResetEvent, WSAEVENT hEvent)
		PASS_PARAMETERS(hEvent);

	MAKE_IMPORT(INT, getnameinfo, const SOCKADDR* pSockaddr, socklen_t SockaddrLength, PCHAR pNodeBuffer, DWORD NodeBufferSize, PCHAR pServiceBuffer, DWORD ServiceBufferSize, INT Flags)
		PASS_PARAMETERS(pSockaddr, SockaddrLength, pNodeBuffer, NodeBufferSize, pServiceBuffer, ServiceBufferSize, Flags);

	MAKE_IMPORT(u_long, ntohl, u_long netlong)
		PASS_PARAMETERS(netlong);

	MAKE_IMPORT(int, recvfrom, SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen)
		PASS_PARAMETERS(s, buf, len, flags, from, fromlen);

	MAKE_IMPORT(unsigned long, inet_addr, const char* cp)
		PASS_PARAMETERS(cp);

	MAKE_IMPORT(u_short, ntohs, u_short netshort)
		PASS_PARAMETERS(netshort);

	MAKE_IMPORT(unsigned __int64, ntohll, unsigned __int64 Value)
		PASS_PARAMETERS(Value);
	IMPORT_END
}
#endif
