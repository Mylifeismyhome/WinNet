#include <Net/Protocol/NTP.h>
#include <Net/Import/Ws2_32.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Protocol)
NET_NAMESPACE_BEGIN(NTP)
NTPRes::NTPRes()
{
	memset(&nframe, NULL, sizeof(NTP_FRAME));
	bvalid = false;
}

NTPRes::NTPRes(NTP_FRAME& frame)
{
	memset(&nframe, NULL, sizeof(NTP_FRAME));
	memcpy_s(&nframe, sizeof(NTP_FRAME), &frame, sizeof(NTP_FRAME));
	bvalid = true;
}

NTP_FRAME& NTPRes::frame()
{
	return nframe;
}

bool NTPRes::valid() const
{
	return bvalid;
}

char* ResolveHostname(const char* name)
{
	WSADATA wsaData;
	auto res = Ws2_32::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[NTP] - WSAStartup has been failed with error: %d"), res);
		return nullptr;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[NTP] - Could not find a usable version of Winsock.dll"));
		Ws2_32::WSACleanup();
		return nullptr;
	}

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result = nullptr;
	const auto dwRetval = Ws2_32::getaddrinfo(name, nullptr, &hints, &result);
	if (dwRetval != NULL)
	{
		LOG_ERROR(CSTRING("[NTP] - Host look up has been failed with error %d"), dwRetval);
		Ws2_32::WSACleanup();
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
		case SOCK_STREAM:
		case SOCK_DGRAM:
			break;

		default:
			// skip
			continue;
		}

		switch (ptr->ai_protocol)
		{
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			break;

		default:
			// skip
			continue;
		}

		if(v6) psockaddrv6 = (struct sockaddr_in6*)ptr->ai_addr;
		else psockaddrv4 = (struct sockaddr_in*)ptr->ai_addr;

		// break out, we have a connectivity we can use
		break;
	}

	if (!psockaddrv4 && !psockaddrv6)
	{
		Ws2_32::freeaddrinfo(result);
		Ws2_32::WSACleanup();
		return nullptr;
	}

	const auto len = psockaddrv6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
	auto buf = ALLOC<char>(len);
	memset(buf, NULL, len);

	if(psockaddrv6) buf = (char*)Ws2_32::inet_ntop(psockaddrv6->sin6_family, &psockaddrv6->sin6_addr, buf, INET6_ADDRSTRLEN);
	else buf = (char*)Ws2_32::inet_ntop(psockaddrv4->sin_family, &psockaddrv4->sin_addr, buf, INET_ADDRSTRLEN);

	Ws2_32::freeaddrinfo(result);
	Ws2_32::WSACleanup();

	return buf;
}

static bool AddrIsV4(const char* addr)
{
	struct sockaddr_in sa;
	if (Ws2_32::inet_pton(AF_INET, addr, &(sa.sin_addr)))
		return true;

	return false;
}

static bool AddrIsV6(const char* addr)
{
	struct sockaddr_in6 sa;
	if (Ws2_32::inet_pton(AF_INET6, addr, &(sa.sin6_addr)))
		return true;

	return false;
}

static NTPRes PerformRequest(const char* addr, u_short port)
{
	WSADATA wsaData;
	auto res = Ws2_32::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[NTP] - WSAStartup has been failed with error: %d"), res);
		return {};
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[NTP] - Could not find a usable version of Winsock.dll"));
		Ws2_32::WSACleanup();
		return {};
	}

	auto v6 = AddrIsV6(addr);
	auto v4 = AddrIsV4(addr);
	if (!v6 && !v4)
	{
		LOG_ERROR(CSTRING("[NTP] - Address is neather IPV4 nor IPV6 Protocol"));
		Ws2_32::WSACleanup();
		return {};
	}

	const auto con = Ws2_32::socket(v6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (con == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP] - Unable to create socket, error code: %d"), WSAGetLastError());
		Ws2_32::WSACleanup();
		return {};
	}

	struct sockaddr* sockaddr = nullptr;
	int slen = NULL;
	if (v4)
	{
		struct sockaddr_in sockaddr4;
		memset((char*)&sockaddr4, 0, sizeof(sockaddr4));
		sockaddr4.sin_family = AF_INET;
		sockaddr4.sin_port = Ws2_32::htons(port);
		sockaddr4.sin_addr.S_un.S_addr = Ws2_32::inet_addr(addr);
		sockaddr = (struct sockaddr*)&sockaddr4;
		slen = static_cast<int>(sizeof(struct sockaddr_in));
	}

	if (v6)
	{
		struct sockaddr_in6 sockaddr6;
		memset((char*)&sockaddr6, 0, sizeof(sockaddr6));
		sockaddr6.sin6_family = AF_INET6;
		sockaddr6.sin6_port = Ws2_32::htons(port);
		res = Ws2_32::inet_pton(AF_INET6, addr, &sockaddr6.sin6_addr);
		if (res != 1)
		{
			LOG_ERROR(CSTRING("[NTP]  - Failure on setting IPV6 Address with error code %d"), res);
			Ws2_32::closesocket(con);
			Ws2_32::WSACleanup();
			return {};
		}
		sockaddr = (struct sockaddr*)&sockaddr6;
		slen = static_cast<int>(sizeof(struct sockaddr_in6));
	}

	if (!sockaddr)
	{
		LOG_ERROR(CSTRING("[NTP]  - Socket is not being valid"));
		Ws2_32::closesocket(con);
		Ws2_32::WSACleanup();
		return {};
	}

	NTP_FRAME frame = {};
	memset(&frame, NULL, sizeof(NTP_FRAME));
	*((char*)&frame) = 0x1b;

	res = Ws2_32::sendto(con, (char*)&frame, sizeof(NTP_FRAME), 0, sockaddr, slen);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP]  - Sending the frame request has been failed with error %d"), WSAGetLastError());
		Ws2_32::closesocket(con);
		Ws2_32::WSACleanup();
		return {};
	}

	res = Ws2_32::recvfrom(con, (char*)&frame, sizeof(NTP_FRAME), 0, sockaddr, &slen);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP]  - Receiving the frame has been failed with error %d"), WSAGetLastError());
		Ws2_32::closesocket(con);
		Ws2_32::WSACleanup();
		return {};
	}

	Ws2_32::closesocket(con);
	Ws2_32::WSACleanup();

	// convert result back to big-endian
	frame.txTm_s = Ws2_32::ntohl(frame.txTm_s);
	frame.txTm_f = Ws2_32::ntohl(frame.txTm_f);
	return { frame };
}

NTPRes Perform(const char* addr, u_short port)
{
	return PerformRequest(addr, port);
}

NTPRes Exec(const char* addr, u_short port)
{
	return PerformRequest(addr, port);
}

NTPRes Run(const char* addr, u_short port)
{
	return PerformRequest(addr, port);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
