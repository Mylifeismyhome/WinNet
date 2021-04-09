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
	IMPORT_DEFINE_FNC_WSAAPI(VOID, freeaddrinfo, PADDRINFOA);
	IMPORT_DEFINE_FNC_WINAPI(int, WSAGetLastError);
	IMPORT_DEFINE_FNC_WINAPI(int, WSAStartup, WORD, LPWSADATA);
	IMPORT_DEFINE_FNC_WINAPI(int, getaddrinfo, PCSTR, PCSTR, const ADDRINFOA*, PADDRINFOA*);
	IMPORT_DEFINE_FNC_WINAPI(SOCKET, socket, int, int, int);
	IMPORT_DEFINE_FNC_WINAPI(int, ioctlsocket, SOCKET, long, u_long*);
	IMPORT_DEFINE_FNC_WINAPI(int, bind, SOCKET, const struct sockaddr FAR*, int);
	IMPORT_DEFINE_FNC_WSAAPI(int, listen, SOCKET, int);
	IMPORT_DEFINE_FNC_WINAPI(int, setsockopt, SOCKET, int, int, const char*, int);
	IMPORT_DEFINE_FNC_WINAPI(int, connect, SOCKET, const sockaddr*, int);
	IMPORT_DEFINE_FNC_WINAPI(SOCKET, accept, SOCKET, sockaddr*, int*);
	IMPORT_DEFINE_FNC_WINAPI(INT, inet_pton, INT, PCSTR, PVOID);
	IMPORT_DEFINE_FNC_WINAPI(u_short, htons, u_short);
	IMPORT_DEFINE_FNC_WINAPI(u_short, htonl, u_short);
	IMPORT_DEFINE_FNC_WINAPI(int, WSARecvFrom, SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, sockaddr*, LPINT, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
	IMPORT_DEFINE_FNC_WINAPI(int, WSAIoctl, SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
	IMPORT_DEFINE_FNC_WINAPI(PCSTR, inet_ntop, INT, const VOID*, PSTR, size_t);
	IMPORT_DEFINE_FNC_WINAPI(int, sendto, SOCKET, const char*, int, int, const sockaddr*, int);
	IMPORT_DEFINE_FNC_WINAPI(BOOL, WSAGetOverlappedResult, SOCKET, LPWSAOVERLAPPED, LPDWORD, BOOL, LPDWORD);
	IMPORT_DEFINE_FNC_WINAPI(BOOL, WSAResetEvent, WSAEVENT);
	IMPORT_DEFINE_FNC_WINAPI(INT, getnameinfo, const SOCKADDR*, socklen_t, PCHAR, DWORD, PCHAR, DWORD, INT);
	IMPORT_DEFINE_FNC_WINAPI(u_long, ntohl, u_long);
	IMPORT_DEFINE_FNC_WINAPI(int, recvfrom, SOCKET, char*, int, int, sockaddr*, int*);
	IMPORT_DEFINE_FNC_WINAPI(unsigned long, inet_addr, const char*);
	IMPORT_END
};