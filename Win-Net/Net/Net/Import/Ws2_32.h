#pragma once
#include <Net/Net/Net.h>

#ifdef NET_DISABLE_IMPORT_WS2_32
#define Ws2_32
#else
#include <Net/Import/Definition.hpp>
#include <winternl.h>

namespace Net
{
	IMPORT_BEGIN_HEAD(Ws2_32);
	IMPORT_DEFINE_FNC_WSAAPI(int, send, SOCKET, const char*, int, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, recv, SOCKET, char*, int, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, closesocket, SOCKET);
	IMPORT_DEFINE_FNC_WSAAPI(int, WSACleanup);
	IMPORT_DEFINE_FNC_WSAAPI(VOID, freeaddrinfo, PADDRINFOA);
	IMPORT_DEFINE_FNC_WSAAPI(int, WSAGetLastError);
	IMPORT_DEFINE_FNC_WSAAPI(int, WSAStartup, WORD, LPWSADATA);
	IMPORT_DEFINE_FNC_WSAAPI(int, getaddrinfo, PCSTR, PCSTR, const ADDRINFOA*, PADDRINFOA*);
	IMPORT_DEFINE_FNC_WSAAPI(SOCKET, socket, int, int, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, ioctlsocket, SOCKET, long, u_long*);
	IMPORT_DEFINE_FNC_WSAAPI(int, bind, SOCKET, const struct sockaddr FAR*, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, listen, SOCKET, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, setsockopt, SOCKET, int, int, const char*, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, connect, SOCKET, const sockaddr*, int);
	IMPORT_DEFINE_FNC_WSAAPI(SOCKET, accept, SOCKET, sockaddr*, int*);
	IMPORT_DEFINE_FNC_WSAAPI(INT, inet_pton, INT, PCSTR, PVOID);
	IMPORT_DEFINE_FNC_WSAAPI(u_short, htons, u_short);
	IMPORT_DEFINE_FNC_WSAAPI(u_short, htonl, u_short);
	IMPORT_DEFINE_FNC_WSAAPI(int, WSARecvFrom, SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, sockaddr*, LPINT, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
	IMPORT_DEFINE_FNC_WSAAPI(int, WSAIoctl, SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
	IMPORT_DEFINE_FNC_WSAAPI(PCSTR, inet_ntop, INT, const VOID*, PSTR, size_t);
	IMPORT_DEFINE_FNC_WSAAPI(int, sendto, SOCKET, const char*, int, int, const sockaddr*, int);
	IMPORT_DEFINE_FNC_WSAAPI(BOOL, WSAGetOverlappedResult, SOCKET, LPWSAOVERLAPPED, LPDWORD, BOOL, LPDWORD);
	IMPORT_DEFINE_FNC_WSAAPI(BOOL, WSAResetEvent, WSAEVENT);
	IMPORT_DEFINE_FNC_WSAAPI(INT, getnameinfo, const SOCKADDR*, socklen_t, PCHAR, DWORD, PCHAR, DWORD, INT);
	IMPORT_DEFINE_FNC_WSAAPI(u_long, ntohl, u_long);
	IMPORT_DEFINE_FNC_WSAAPI(int, recvfrom, SOCKET, char*, int, int, sockaddr*, int*);
	IMPORT_DEFINE_FNC_WSAAPI(unsigned long, inet_addr, const char*);
	IMPORT_DEFINE_FNC_WSAAPI(u_short, ntohs, u_short);
	IMPORT_DEFINE_FNC_WSAAPI(unsigned __int64, ntohll, unsigned __int64);
	IMPORT_END
};
#endif
