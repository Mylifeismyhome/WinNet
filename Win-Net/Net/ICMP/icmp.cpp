#include <ICMP/icmp.h>

static int SetTtl(const SOCKET s, int ttl, const int gAddressFamily)

{

	int     optlevel, option, rc;



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

	{

		rc = SOCKET_ERROR;

	}

	if (rc == NO_ERROR)

	{

		rc = setsockopt(s, optlevel, option, (char*)&ttl, sizeof(ttl));

	}

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

	icmp_hdr->icmp_timestamp = GetTickCount();



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

	{

		cksum += *(UCHAR*)buffer;

	}

	cksum = (cksum >> 16) + (cksum & 0xffff);

	cksum += (cksum >> 16);

	return (USHORT)(~cksum);

}

static int PostRecvfrom(const SOCKET s, char* buf, const int buflen, SOCKADDR* from, int* fromlen, WSAOVERLAPPED* ol)

{

	WSABUF  wbuf;

	DWORD   flags, bytes;

	int     rc;



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
	int     hdrlen = 0, routes = 0, i;



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

		//    header then there are options present. Find them and print them.

		if (hdrlen > sizeof(IPV4_HDR))

		{

			v4opt = (IPV4_OPTION_HDR*)(buf + sizeof(IPV4_HDR));

			routes = (v4opt->opt_ptr / sizeof(ULONG)) - 1;

			for (i = 0; i < routes; i++)

			{

				hop.sin_addr.s_addr = v4opt->opt_addr[i];

				// Print the route

				if (i == 0)

					printf(CSTRING("    Route: "));

				else

					printf(CSTRING("           "));

				PrintAddress((SOCKADDR*)&hop, sizeof(hop));



				if (i < routes - 1)

					printf(CSTRING(" ->\n"));

				else

					printf(CSTRING("\n"));

			}

		}

	}
}

static void SetIcmpSequence(char* buf, const int gAddressFamily)

{

	ULONG    sequence = 0;



	sequence = GetTickCount();

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

static USHORT ComputeIcmp6PseudoHeaderChecksum(const SOCKET s, char* icmppacket, const int icmplen, struct addrinfo* dest)

{

	SOCKADDR_STORAGE localif;

	DWORD            bytes;

	char             tmp[65535], * ptr = NULL, proto = 0, zero = 0;

	int              rc, total, length, i;



	// Find out which local interface for the destination

	rc = WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY, dest->ai_addr, dest->ai_addrlen,

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

	memcpy(ptr, &((SOCKADDR_IN6*)dest->ai_addr)->sin6_addr, sizeof(struct in6_addr));

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

static void ComputeIcmpChecksum(const SOCKET s, char* buf, const int packetlen, struct addrinfo* dest, const int gAddressFamily)

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

		icmpv6->icmp6_checksum = ComputeIcmp6PseudoHeaderChecksum(s, buf, packetlen, dest);

	}

}

struct addrinfo* ResolveAddress(char* addr, char* port, const int af, const int type, const int proto)
{

	struct addrinfo hints,

		* res = NULL;

	int             rc;



	memset(&hints, 0, sizeof(hints));

	hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);

	hints.ai_family = af;

	hints.ai_socktype = type;

	hints.ai_protocol = proto;



	rc = getaddrinfo(addr, port, &hints, &res);

	if (rc != 0)
		return nullptr;

	return res;
}

int PrintAddress(SOCKADDR* sa, const int salen)

{

	char    host[NI_MAXHOST], serv[NI_MAXSERV];

	int     hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc;



	rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);

	if (rc != 0)
		return rc;

	return NO_ERROR;

}

ICMP::ICMP(const char* _address)
{
	_socket = INVALID_SOCKET;
	_latency = NULL;

	sprintf_s(this->_address, _address);

	gAddressFamily = AF_UNSPEC;
	gDataSize = DEFAULT_DATA_SIZE;
	bRecordRoute = FALSE;
	gDestination = nullptr;
}

void ICMP::execute()
{
	WSADATA            wsd;
	WSAOVERLAPPED      recvol;
	char* icmpbuf = nullptr;
	auto recvbuf = new char[0xFFFF];
	struct addrinfo* dest = nullptr, * local = nullptr;
	IPV4_OPTION_HDR    ipopt;
	SOCKADDR_STORAGE   from;
	DWORD              bytes, flags;
	int                packetlen = 0, fromlen, rc;
	const auto recvbuflen = 0xFFFF;
	lt time = 0;

	// Load Winsock

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		return;
	}

	dest = ResolveAddress((char*)CSTRING("85.214.56.162"), (char*)CSTRING("0"), gAddressFamily, 0, 0);
	if (!dest)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		return;
	}

	int gProtocol = IPPROTO_ICMP;

	gAddressFamily = dest->ai_family;
	if (gAddressFamily == AF_INET)
		gProtocol = IPPROTO_ICMP;
	else if (gAddressFamily == AF_INET6)
		gProtocol = IPPROTO_ICMP6;

	// Get the bind address
	local = ResolveAddress(nullptr, (char*)CSTRING("0"), gAddressFamily, 0, 0);
	if (!local)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		return;
	}

	_socket = socket(gAddressFamily, SOCK_RAW, gProtocol);

	if (_socket == INVALID_SOCKET)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		return;
	}

	int gTtl = 128;

	SetTtl(_socket, gTtl, gAddressFamily);

	// Figure out the size of the ICMP header and payload
	if (gAddressFamily == AF_INET)
		packetlen += sizeof(ICMP_HDR);
	else if (gAddressFamily == AF_INET6)
		packetlen += sizeof(ICMPV6_HDR) + sizeof(ICMPV6_ECHO_REQUEST);

	// Add in the data size
	packetlen += gDataSize;

	icmpbuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, packetlen);
	if (!icmpbuf)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return;
	}

	// Initialize the ICMP headers
	if (gAddressFamily == AF_INET)
	{
		if (bRecordRoute)
		{
			ZeroMemory(&ipopt, sizeof(ipopt));
			ipopt.opt_code = IP_RECORD_ROUTE; // record route option
			ipopt.opt_ptr = 4;                 // point to the first addr offset
			ipopt.opt_len = 39;              // length of option header

			rc = setsockopt(_socket, IPPROTO_IP, IP_OPTIONS, (char*)&ipopt, sizeof(ipopt));

			if (rc == SOCKET_ERROR)
			{
				if (recvbuf)
				{
					delete[] recvbuf;
					recvbuf = nullptr;
				}

				HeapFree(GetProcessHeap(), 0, icmpbuf);
				return;
			}
		}

		InitIcmpHeader(icmpbuf, gDataSize);
	}
	else if (gAddressFamily == AF_INET6)
		InitIcmp6Header(icmpbuf, gDataSize);

	rc = bind(_socket, local->ai_addr, local->ai_addrlen);

	if (rc == SOCKET_ERROR)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return;
	}

	memset(&recvol, 0, sizeof(recvol));

	recvol.hEvent = WSACreateEvent();

	fromlen = sizeof(from);

	PostRecvfrom(_socket, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);

	PrintAddress(dest->ai_addr, dest->ai_addrlen);

	SetIcmpSequence(icmpbuf, gAddressFamily);

	ComputeIcmpChecksum(_socket, icmpbuf, packetlen, dest, gAddressFamily);

#ifdef _WIN64
	time = GetTickCount64();
#else
	time = GetTickCount();
#endif

	rc = sendto(_socket, icmpbuf, packetlen, 0, dest->ai_addr, dest->ai_addrlen);

	if (rc == SOCKET_ERROR)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return;
	}

	// Waite for a response
	rc = WaitForSingleObject((HANDLE)recvol.hEvent, DEFAULT_RECV_TIMEOUT);

	if (rc == WAIT_FAILED)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return;
	}

	if (rc == WAIT_TIMEOUT)
	{
		if (recvbuf)
		{
			delete[] recvbuf;
			recvbuf = nullptr;
		}

		HeapFree(GetProcessHeap(), 0, icmpbuf);
		return;
	}

	rc = WSAGetOverlappedResult(_socket, &recvol, &bytes, FALSE, &flags);

#ifdef _WIN64
	time = GetTickCount64() - time;
#else
	time = GetTickCount() - time;
#endif

	WSAResetEvent(recvol.hEvent);
	PrintAddress((SOCKADDR*)&from, fromlen);

	_latency = time;

	PrintPayload(recvbuf, bytes, gAddressFamily);
	fromlen = sizeof(from);
	PostRecvfrom(_socket, recvbuf, recvbuflen, (SOCKADDR*)&from, &fromlen, &recvol);

	if (recvbuf)
	{
		delete[] recvbuf;
		recvbuf = nullptr;
	}
	
	freeaddrinfo(dest);
	freeaddrinfo(local);

	if (_socket != INVALID_SOCKET)
		closesocket(_socket);

	HeapFree(GetProcessHeap(), 0, icmpbuf);

	WSACleanup();
}

lt ICMP::getLatency() const
{
	return _latency;
}