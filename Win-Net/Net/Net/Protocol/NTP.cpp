#include <Net/Protocol/NTP.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Protocol)
NET_NAMESPACE_BEGIN(NTP)
NTPRes::NTPRes()
{
	memset(&nframe, NULL, sizeof(NTP_FRAME));
	bvalid = false;
}

NTPRes::NTPRes(NTP_FRAME frame)
{
	memset(&nframe, NULL, sizeof(NTP_FRAME));
	memcpy_s(&nframe, sizeof(NTP_FRAME), &frame, sizeof(NTP_FRAME));
	bvalid = true;
}

NTP_FRAME  NTPRes::frame() const
{
	return nframe;
}

bool NTPRes::valid() const
{
	return bvalid;
}

static NTPRes PerformRequest(const char* addr, u_short port)
{
	WSADATA wsaData;
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[NTP] - WSAStartup has been failed with error: %d"), res);
		return {};
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[NTP] - Could not find a usable version of Winsock.dll"));
		WSACleanup();
		return {};
	}

	const auto con = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (con == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP] - Unable to create socket, error code: %d"), WSAGetLastError());
		WSACleanup();
		return {};
	}

	struct sockaddr_in sockaddr;
	memset((char*)&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.S_un.S_addr = inet_addr(addr);

	NTP_FRAME frame = {};
	memset(&frame, NULL, sizeof(NTP_FRAME));
	*((char*)&frame) = 0x1b;

	auto slen = static_cast<int>(sizeof(struct sockaddr_in));
	res = sendto(con, (char*)&frame, sizeof(NTP_FRAME), 0, (struct sockaddr*)&sockaddr, slen);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP]  - Sending the frame request has been failed with error %d"), res);
		closesocket(con);
		WSACleanup();
		return {};
	}

	res = recvfrom(con, (char*)&frame, sizeof(NTP_FRAME), 0, (struct sockaddr*)&sockaddr, &slen);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP]  - Receiving the frame has been failed with error %d"), res);
		closesocket(con);
		WSACleanup();
		return {};
	}

	closesocket(con);
	WSACleanup();

	// convert result back to big-endian
	frame.txTm_s = ntohl(frame.txTm_s);
	frame.txTm_f = ntohl(frame.txTm_f);
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