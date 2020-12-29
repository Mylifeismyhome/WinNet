#include <Net/Protocol/ICMP.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Protocol)
NET_NAMESPACE_BEGIN(ICMP)
static bool AddrIsV4(const char* addr)
{
	struct sockaddr_in sa;
	if (inet_pton(AF_INET, addr, &(sa.sin_addr)))
		return true;

	return false;
}

static bool AddrIsV6(const char* addr)
{
	struct sockaddr_in6 sa;
	if (inet_pton(AF_INET6, addr, &(sa.sin6_addr)))
		return true;

	return false;
}

int PrintAddress(SOCKADDR* sa, const int salen)
{
	char  host[NI_MAXHOST], serv[NI_MAXSERV];
	int  hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc;

	rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
	if (rc != 0)
		return rc;

	return NO_ERROR;
}

static int SetTtl(const SOCKET s, int ttl, const int gAddressFamily)
{
	int optlevel, option, rc;
	rc = NO_ERROR;

	if (gAddressFamily == AF_INET)
	{
		optlevel = IPPROTO_IP;
		option = IP_TTL;
	}
	else if (gAddressFamily == AF_INET6)
	{
		optlevel = IPPROTO_IPV6;
		option = IPV6_UNICAST_HOPS;
	}
	else
		rc = SOCKET_ERROR;

	if (rc == NO_ERROR)
		rc = setsockopt(s, optlevel, option, (char*)&ttl, sizeof(ttl));

	return rc;
}

static void InitIcmpHeader(char* buf, const int datasize)
{
	ICMP_HDR* icmp_hdr = NULL;
	char* datapart = NULL;

	icmp_hdr = (ICMP_HDR*)buf;
	icmp_hdr->icmp_type = ICMPV4_ECHO_REQUEST_TYPE;        // request an ICMP echo
	icmp_hdr->icmp_code = ICMPV4_ECHO_REQUEST_CODE;
	icmp_hdr->icmp_id = (USHORT)GetCurrentProcessId();
	icmp_hdr->icmp_checksum = 0;
	icmp_hdr->icmp_sequence = 0;

#ifdef _WIN64
	icmp_hdr->icmp_timestamp = GetTickCount64();
#else
	icmp_hdr->icmp_timestamp = GetTickCount();
#endif

	datapart = buf + sizeof(ICMP_HDR);

	// Place some junk in the buffer.
	memset(datapart, 'E', datasize);
}

static int InitIcmp6Header(char* buf, const int datasize)
{
	ICMPV6_HDR* icmp6_hdr = NULL;
	ICMPV6_ECHO_REQUEST* icmp6_req = NULL;
	char* datapart = NULL;

	// Initialize the ICMP6 header fields
	icmp6_hdr = (ICMPV6_HDR*)buf;
	icmp6_hdr->icmp6_type = ICMPV6_ECHO_REQUEST_TYPE;
	icmp6_hdr->icmp6_code = ICMPV6_ECHO_REQUEST_CODE;
	icmp6_hdr->icmp6_checksum = 0;

	// Initialize the echo request fields
	icmp6_req = (ICMPV6_ECHO_REQUEST*)(buf + sizeof(ICMPV6_HDR));
	icmp6_req->icmp6_echo_id = (USHORT)GetCurrentProcessId();
	icmp6_req->icmp6_echo_sequence = 0;

	datapart = (char*)buf + sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);
	memset(datapart, '$', datasize);

	return (sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST));
}

static USHORT checksum(USHORT* buffer, int size)
{
	unsigned long cksum = 0;

	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}

	if (size)
		cksum += *(UCHAR*)buffer;

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT)(~cksum);
}

static int PostRecvfrom(const SOCKET s, char* buf, const int buflen, SOCKADDR* from, int* fromlen, WSAOVERLAPPED* ol)
{
	WSABUF wbuf;
	DWORD flags, bytes;
	int rc;

	wbuf.buf = buf;
	wbuf.len = buflen;
	flags = 0;
	rc = WSARecvFrom(s, &wbuf, 1, &bytes, &flags, from, fromlen, ol, NULL);
	if (rc == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return SOCKET_ERROR;
	}

	return NO_ERROR;
}

static void PrintPayload(char* buf, int bytes, const int gAddressFamily)
{
	int hdrlen = 0, routes = 0, i;
	if (gAddressFamily == AF_INET)
	{
		SOCKADDR_IN      hop;
		IPV4_OPTION_HDR* v4opt = NULL;
		IPV4_HDR* v4hdr = NULL;

		hop.sin_family = (USHORT)gAddressFamily;
		hop.sin_port = 0;

		v4hdr = (IPV4_HDR*)buf;
		hdrlen = (v4hdr->ip_verlen & 0x0F) * 4;

		// If the header length is greater than the size of the basic IPv4
		//  header then there are options present. Find them and print them.
		if (hdrlen > sizeof(IPV4_HDR))
		{
			v4opt = (IPV4_OPTION_HDR*)(buf + sizeof(IPV4_HDR));
			routes = (v4opt->opt_ptr / sizeof(ULONG)) - 1;
			for (i = 0; i < routes; i++)
			{
				hop.sin_addr.s_addr = v4opt->opt_addr[i];
				PrintAddress((SOCKADDR*)&hop, sizeof(hop));
			}
		}
	}
}

static void SetIcmpSequence(char* buf, const int gAddressFamily)
{
	DWORD sequence = 0;
#ifdef _WIN64
	sequence = GetTickCount64();
#else
	sequence = GetTickCount();
#endif

	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR* icmpv4 = NULL;
		icmpv4 = (ICMP_HDR*)buf;
		icmpv4->icmp_sequence = (USHORT)sequence;
	}
	else if (gAddressFamily == AF_INET6)
	{
		ICMPV6_HDR* icmpv6 = NULL;
		ICMPV6_ECHO_REQUEST* req6 = NULL;
		icmpv6 = (ICMPV6_HDR*)buf;
		req6 = (ICMPV6_ECHO_REQUEST*)(buf + sizeof(ICMPV6_HDR));
		req6->icmp6_echo_sequence = (USHORT)sequence;
	}
}

static USHORT ComputeIcmp6PseudoHeaderChecksum(const SOCKET s, char* icmppacket, const int icmplen, struct sockaddr* dest, const int destlen)
{
	SOCKADDR_STORAGE localif;
	DWORD bytes;
	char  tmp[65535], * ptr = NULL, proto = 0, zero = 0;
	int rc, total, length, i;

	// Find out which local interface for the destination
	rc = WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY, dest, destlen,
		(SOCKADDR*)&localif, sizeof(localif), &bytes, nullptr, nullptr);

	if (rc == SOCKET_ERROR)
		return -1;

	// We use a temporary buffer to calculate the pseudo header.
	ptr = tmp;
	total = 0;

	// Copy source address
	memcpy(ptr, &((SOCKADDR_IN6*)&localif)->sin6_addr, sizeof(struct in6_addr));
	ptr += sizeof(struct in6_addr);
	total += sizeof(struct in6_addr);

	// Copy destination address
	memcpy(ptr, &((SOCKADDR_IN6*)dest)->sin6_addr, sizeof(struct in6_addr));
	ptr += sizeof(struct in6_addr);
	total += sizeof(struct in6_addr);

	// Copy ICMP packet length
	length = htonl(icmplen);

	memcpy(ptr, &length, sizeof(length));
	ptr += sizeof(length);
	total += sizeof(length);

	// Zero the 3 bytes
	memset(ptr, 0, 3);
	ptr += 3;
	total += 3;

	// Copy next hop header
	proto = IPPROTO_ICMP6;
	memcpy(ptr, &proto, sizeof(proto));
	ptr += sizeof(proto);
	total += sizeof(proto);

	// Copy the ICMP header and payload
	memcpy(ptr, icmppacket, icmplen);
	ptr += icmplen;
	total += icmplen;
	for (i = 0; i < icmplen % 2; i++)
	{
		*ptr = 0;
		ptr++;
		total++;
	}
	return checksum((USHORT*)tmp, total);
}

static void ComputeIcmpChecksum(const SOCKET s, char* buf, const int packetlen, struct sockaddr* dest, const int destlen, const int gAddressFamily)
{
	if (gAddressFamily == AF_INET)
	{
		ICMP_HDR* icmpv4 = NULL;
		icmpv4 = (ICMP_HDR*)buf;
		icmpv4->icmp_checksum = 0;
		icmpv4->icmp_checksum = checksum((USHORT*)buf, packetlen);
	}
	else if (gAddressFamily == AF_INET6)
	{
		ICMPV6_HDR* icmpv6 = NULL;
		icmpv6 = (ICMPV6_HDR*)buf;
		icmpv6->icmp6_checksum = 0;
		icmpv6->icmp6_checksum = ComputeIcmp6PseudoHeaderChecksum(s, buf, packetlen, dest, destlen);
	}
}

struct addrinfo* ResolveLocalAddress(const int af)
{
	struct addrinfo hints, * res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = af;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;

	const auto 	rc = getaddrinfo(nullptr, (char*)CSTRING("0"), &hints, &res);
	if (rc != 0)
		return nullptr;

	return res;
}

char* ResolveHostname(const char* name)
{
	WSADATA wsaData;
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[ICMP] - WSAStartup has been failed with error: %d"), res);
		return nullptr;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[ICMP] - Could not find a usable version of Winsock.dll"));
		WSACleanup();
		return nullptr;
	}

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result = nullptr;
	const auto dwRetval = getaddrinfo(name, nullptr, &hints, &result);
	if (dwRetval != NULL)
	{
		LOG_ERROR(CSTRING("[ICMP] - Host look up has been failed with error %d"), dwRetval);
		WSACleanup();
		return nullptr;
	}

	struct sockaddr_in* psockaddrv4 = nullptr;
	struct sockaddr_in6* psockaddrv6 = nullptr;
	struct addrinfo* ptr = nullptr;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		bool v6 = false;
		switch (ptr->ai_family)
		{
		case AF_INET:
			break;

		case AF_INET6:
			v6 = true;
			break;

		default:
			// skip
			continue;
		}

		switch (ptr->ai_socktype)
		{
		case SOCK_RAW:
			break;

		default:
			// skip
			continue;
		}

		switch (ptr->ai_protocol)
		{
		case IPPROTO_ICMP:
			break;

		default:
			// skip
			continue;
		}

		if (v6) psockaddrv6 = (struct sockaddr_in6*)ptr->ai_addr;
		else psockaddrv4 = (struct sockaddr_in*)ptr->ai_addr;

		// break out, we have a connectivity we can use
		break;
	}

	if (!psockaddrv4 && !psockaddrv6)
	{
		freeaddrinfo(result);
		WSACleanup();
		return nullptr;
	}

	const auto len = psockaddrv6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
	auto buf = ALLOC<char>(len);
	memset(buf, NULL, len);

	if (psockaddrv6) buf = (char*)inet_ntop(psockaddrv6->sin6_family, &psockaddrv6->sin6_addr, buf, INET6_ADDRSTRLEN);
	else buf = (char*)inet_ntop(psockaddrv4->sin_family, &psockaddrv4->sin_addr, buf, INET_ADDRSTRLEN);

	freeaddrinfo(result);
	WSACleanup();

	return buf;
}

static lt PerformRequest(const char* addr, const bool bRecordRoute)
{
	WSADATA wsaData;
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[ICMP] - WSAStartup has been failed with error: %d"), res);
		return INVALID_SIZE;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[ICMP] - Could not find a usable version of Winsock.dll"));
		WSACleanup();
		return INVALID_SIZE;
	}

	auto v6 = AddrIsV6(addr);
	auto v4 = AddrIsV4(addr);
	if (!v6 && !v4)
	{
		LOG_ERROR(CSTRING("[ICMP] - Address is neather IPV4 nor IPV6 Protocol"));
		WSACleanup();
		return INVALID_SIZE;
	}

	const auto con = socket(v4 ? AF_INET : AF_INET6, SOCK_RAW, IPPROTO_ICMP);
	if (con == INVALID_SOCKET)
	{
		return INVALID_SIZE;
	}

	struct sockaddr* sockaddr = nullptr;
	struct addrinfo* localsockaddr = ResolveLocalAddress(v4 ? AF_INET : AF_INET6);
	if (!localsockaddr)
	{
		return INVALID_SIZE;
	}

	int slen = NULL;
	if (v4)
	{
		struct sockaddr_in sockaddr4;
		memset((char*)&sockaddr4, 0, sizeof(sockaddr4));
		sockaddr4.sin_family = AF_INET;
		sockaddr4.sin_port = htons(0);
		sockaddr4.sin_addr.S_un.S_addr = inet_addr(addr);
		sockaddr = (struct sockaddr*)&sockaddr4;
		slen = static_cast<int>(sizeof(struct sockaddr_in));
	}

	if (v6)
	{
		struct sockaddr_in6 sockaddr6;
		memset((char*)&sockaddr6, 0, sizeof(sockaddr6));
		sockaddr6.sin6_family = AF_INET6;
		sockaddr6.sin6_port = htons(0);
		res = inet_pton(AF_INET6, addr, &sockaddr6.sin6_addr);
		if (res != 1)
		{
			LOG_ERROR(CSTRING("[ICMP]  - Failure on setting IPV6 Address with error code %d"), res);
			closesocket(con);
			WSACleanup();
			return INVALID_SIZE;
		}
		sockaddr = (struct sockaddr*)&sockaddr6;
		slen = static_cast<int>(sizeof(struct sockaddr_in6));
	}

	if (!sockaddr)
	{
		LOG_ERROR(CSTRING("[ICMP]  - Socket is not being valid"));
		closesocket(con);
		WSACleanup();
		return INVALID_SIZE;
	}

	char* icmpbuf = nullptr;
	char recvbuf[0xFFFF];

	SOCKADDR_STORAGE   from;
	DWORD              bytes, flags;
	int                packetlen = 0, fromlen, rc;
	const auto recvbuflen = 0xFFFF;
	lt time = 0;

	SetTtl(con, DEFAULT_TTL, v4 ? AF_INET : AF_INET6);

	// Figure out the size of the ICMP header and payload
	if (v4)
		packetlen += sizeof(ICMP_HDR);
	else if (v6)
		packetlen += sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

	// Add in the data size
	packetlen += DEFAULT_DATA_SIZE;

	icmpbuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, packetlen);
	if (!icmpbuf)
	{
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return INVALID_SIZE;
	}

	// Initialize the ICMP headers
	if (v4)
	{
		if (bRecordRoute)
		{
			IPV4_OPTION_HDR ipopt;
			ZeroMemory(&ipopt, sizeof(ipopt));
			ipopt.opt_code = IP_RECORD_ROUTE; // record route option
			ipopt.opt_ptr = 4;                 // point to the first addr offset
			ipopt.opt_len = 39;              // length of option header

			rc = setsockopt(con, IPPROTO_IP, IP_OPTIONS, (char*)&ipopt, sizeof(ipopt));
			if (rc == SOCKET_ERROR)
			{
				HeapFree(GetProcessHeap(), 0, icmpbuf);
				return INVALID_SIZE;
			}
		}

		InitIcmpHeader(icmpbuf, DEFAULT_DATA_SIZE);
	}
	else if (v6)
		InitIcmp6Header(icmpbuf, DEFAULT_DATA_SIZE);

	rc = bind(con, localsockaddr->ai_addr, localsockaddr->ai_addrlen);
	if (rc == SOCKET_ERROR)
	{
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return INVALID_SIZE;
	}

	WSAOVERLAPPED recvol;
	memset(&recvol, 0, sizeof(recvol));
	recvol.hEvent = WSACreateEvent();
	fromlen = sizeof(from);

	PostRecvfrom(con, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);
	PrintAddress(sockaddr, slen);
	SetIcmpSequence(icmpbuf, v4 ? AF_INET : AF_INET6);
	ComputeIcmpChecksum(con, icmpbuf, packetlen, sockaddr, slen, v4 ? AF_INET : AF_INET6);

#ifdef _WIN64
	time = GetTickCount64();
#else
	time = GetTickCount();
#endif

	rc = sendto(con, icmpbuf, packetlen, 0, sockaddr, slen);
	if (rc == SOCKET_ERROR)
	{
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return INVALID_SIZE;
	}

	// Waite for a response
	rc = WaitForSingleObject((HANDLE)recvol.hEvent, DEFAULT_RECV_TIMEOUT);
	if (rc == WAIT_FAILED)
	{
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return INVALID_SIZE;
	}

	if (rc == WAIT_TIMEOUT)
	{
		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return INVALID_SIZE;
	}
	rc = WSAGetOverlappedResult(con, &recvol, &bytes, FALSE, &flags);

#ifdef _WIN64
	time = GetTickCount64() - time;
#else
	time = GetTickCount() - time;
#endif

	WSAResetEvent(recvol.hEvent);
	PrintAddress((SOCKADDR*)&from, fromlen);

	PrintPayload(recvbuf, bytes, v4 ? AF_INET : AF_INET6);
	fromlen = sizeof(from);
	PostRecvfrom(con, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);

	if (con != INVALID_SOCKET)
		closesocket(con);

	HeapFree(GetProcessHeap(), 0, icmpbuf);

	freeaddrinfo(localsockaddr);

	WSACleanup();
	return time;
}

lt Perform(const char* addr, const bool bRecordRoute)
{
	return PerformRequest(addr, bRecordRoute);
}

lt Exec(const char* addr, const bool bRecordRoute)
{
	return PerformRequest(addr, bRecordRoute);
}

lt Run(const char* addr, const bool bRecordRoute)
{
	return PerformRequest(addr, bRecordRoute);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END