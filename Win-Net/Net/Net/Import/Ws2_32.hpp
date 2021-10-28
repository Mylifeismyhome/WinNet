#pragma once
#ifndef BUILD_LINUX
#include "ImportResolver.h"

#ifdef NET_DISABLE_IMPORT_WS2_32
#define Ws2_32
#else
#define IMPORT_NAME Ws2_32

RESOLVE_IMPORT_BEGIN;
DEFINE_IMPORT(int, send, SOCKET s, const char* buf, int len, int flags);
MAKE_IMPORT(s, buf, len, flags);

DEFINE_IMPORT(int, recv, SOCKET s, char* buf, int len, int flags);
MAKE_IMPORT(s, buf, len, flags);

DEFINE_IMPORT(int, closesocket, SOCKET s);
MAKE_IMPORT(s);

DEFINE_IMPORT(int, WSACleanup);
MAKE_IMPORT();

DEFINE_IMPORT(VOID, freeaddrinfo, PADDRINFOA pAddrInfo);
MAKE_IMPORT(pAddrInfo);

DEFINE_IMPORT(int, WSAGetLastError);
MAKE_IMPORT();

DEFINE_IMPORT(int, WSAStartup, WORD wVersionRequired, LPWSADATA lpWSAData);
MAKE_IMPORT(wVersionRequired, lpWSAData);

DEFINE_IMPORT(int, getaddrinfo, PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);
MAKE_IMPORT(pNodeName, pServiceName, pHints, ppResult);

DEFINE_IMPORT(SOCKET, socket, int af, int type, int protocol);
MAKE_IMPORT(af, type, protocol);

DEFINE_IMPORT(int, ioctlsocket, SOCKET s, long cmd, u_long* argp);
MAKE_IMPORT(s, cmd, argp);

DEFINE_IMPORT(int, bind, SOCKET s, const struct sockaddr FAR* addr, int namelen);
MAKE_IMPORT(s, addr, namelen);

DEFINE_IMPORT(int, listen, SOCKET s, int backlog);
MAKE_IMPORT(s, backlog);

DEFINE_IMPORT(int, setsockopt, SOCKET s, int level, int optname, const char* optval, int optlen);
MAKE_IMPORT(s, level, optname, optval, optlen);

DEFINE_IMPORT(int, connect, SOCKET s, const sockaddr* name, int namelen);
MAKE_IMPORT(s, name, namelen);

DEFINE_IMPORT(SOCKET, accept, SOCKET s, sockaddr* addr, int* addrlen);
MAKE_IMPORT(s, addr, addrlen);

DEFINE_IMPORT(INT, inet_pton, INT Family, PCSTR pszAddrString, PVOID pAddrBuf);
MAKE_IMPORT(Family, pszAddrString, pAddrBuf);

DEFINE_IMPORT(u_short, htons, u_short hostshort);
MAKE_IMPORT(hostshort);

DEFINE_IMPORT(u_short, htonl, u_short hostlong);
MAKE_IMPORT(hostlong);

DEFINE_IMPORT(int, WSARecvFrom, SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, sockaddr* lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
MAKE_IMPORT(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine);

DEFINE_IMPORT(int, WSAIoctl, SOCKET s, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
MAKE_IMPORT(s, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);

DEFINE_IMPORT(PCSTR, inet_ntop, INT Family, const VOID* pAddr, PSTR pStringBuf, size_t StringBufSize);
MAKE_IMPORT(Family, pAddr, pStringBuf, StringBufSize);

DEFINE_IMPORT(int, sendto, SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen);
MAKE_IMPORT(s, buf, len, flags, to, tolen);

DEFINE_IMPORT(BOOL, WSAGetOverlappedResult, SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer, BOOL fWait, LPDWORD lpdwFlags);
MAKE_IMPORT(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags);

DEFINE_IMPORT(BOOL, WSAResetEvent, WSAEVENT hEvent);
MAKE_IMPORT(hEvent);

DEFINE_IMPORT(INT, getnameinfo, const SOCKADDR* pSockaddr, socklen_t SockaddrLength, PCHAR pNodeBuffer, DWORD NodeBufferSize, PCHAR pServiceBuffer, DWORD ServiceBufferSize, INT Flags)
MAKE_IMPORT(pSockaddr, SockaddrLength, pNodeBuffer, NodeBufferSize, pServiceBuffer, ServiceBufferSize, Flags);

DEFINE_IMPORT(u_long, ntohl, u_long netlong)
MAKE_IMPORT(netlong);

DEFINE_IMPORT(int, recvfrom, SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen)
MAKE_IMPORT(s, buf, len, flags, from, fromlen);

DEFINE_IMPORT(unsigned long, inet_addr, const char* cp)
MAKE_IMPORT(cp);

DEFINE_IMPORT(u_short, ntohs, u_short netshort)
MAKE_IMPORT(netshort);

DEFINE_IMPORT(unsigned __int64, ntohll, unsigned __int64 Value)
MAKE_IMPORT(Value);
RESOLVE_IMPORT_END;
#endif
#endif
