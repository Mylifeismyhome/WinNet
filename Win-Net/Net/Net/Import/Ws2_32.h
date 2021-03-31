#pragma once
#include <Net/Net/Net.h>
#include <Net/Import/Definition.hpp>
#include <winternl.h>

namespace Net
{
	IMPORT_BEGIN(Ws2_32);
	IMPORT_DEFINE_FNC_WINAPI(int, send, SOCKET, const char*, int, int);
	IMPORT_DEFINE_FNC_WINAPI(int, recv, SOCKET, const char*, int, int);
	IMPORT_DEFINE_FNC_WINAPI(int, closesocket, SOCKET);
	IMPORT_DEFINE_FNC_WINAPI(int, WSACleanup);
	IMPORT_DEFINE_FNC_WINAPI(VOID, freeaddrinfo, PADDRINFOA);
	IMPORT_DEFINE_FNC_WINAPI(int, WSAGetLastError);
	IMPORT_DEFINE_FNC_WINAPI(int, WSAStartup, WORD, LPWSADATA);
	IMPORT_DEFINE_FNC_WINAPI(int, getaddrinfo, PCSTR, PCSTR, const ADDRINFOA*, PADDRINFOA*);
	IMPORT_DEFINE_FNC_WINAPI(SOCKET, socket, int, int, int);
	IMPORT_DEFINE_FNC_WINAPI(int, ioctlsocket, SOCKET, long, u_long*);
	IMPORT_DEFINE_FNC_WINAPI(int, bind, SOCKET, const struct sockaddr FAR*, int);
	IMPORT_DEFINE_FNC_WINAPI(int, listen, SOCKET, int);
	IMPORT_DEFINE_FNC_WINAPI(int, setsockopt, SOCKET, int, int, const char*, int);
	IMPORT_DEFINE_FNC_WINAPI(int, connect, SOCKET, const sockaddr*, int);
	IMPORT_DEFINE_FNC_WINAPI(SOCKET, accept, SOCKET, sockaddr*, int*);
	IMPORT_END
};