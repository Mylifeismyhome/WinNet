#include <NetClient/Client.h>
#include <Net/Import/Kernel32.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Client)
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

NET_TIMER(CalcLatency)
{
	const auto client = (Client*)param;
	if (!client) NET_STOP_TIMER;

	if (!client->IsConnected()) NET_CONTINUE_TIMER;

	client->network.bLatency = true;
	client->network.latency = Net::Protocol::ICMP::Exec(client->GetServerAddress());
	client->network.bLatency = false;

	Timer::SetTime(client->network.hCalcLatency, client->Isset(NET_OPT_INTERVAL_LATENCY) ? client->GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY);
	NET_CONTINUE_TIMER;
}

NET_TIMER(NTPSyncClock)
{
	const auto client = (Client*)param;
	if (!client) NET_STOP_TIMER;

	if (!client->IsConnected()) NET_CONTINUE_TIMER;

	const auto time = Net::Protocol::NTP::Exec(client->Isset(NET_OPT_NTP_HOST) ? client->GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
		client->Isset(NET_OPT_NTP_PORT) ? client->GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

	if (!time.valid())
	{
		LOG_ERROR(CSTRING("[NET] - critical failure on calling NTP host"));
		NET_CONTINUE_TIMER;
	}

	client->network.curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

	Timer::SetTime(client->network.hSyncClockNTP, client->Isset(NET_OPT_NTP_SYNC_INTERVAL) ? client->GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL);
	NET_CONTINUE_TIMER;
}

Client::Client()
{
	Net::Codes::NetLoadErrorCodes();
	SetSocket(INVALID_SOCKET);
	SetServerAddress(CSTRING(""));
	SetServerPort(NULL);
	SetConnected(false);
	NeedExit = FALSE;
	bAccomplished = FALSE;
	optionBitFlag = NULL;
	socketOptionBitFlag = NULL;
}

Client::~Client()
{
}

NET_THREAD(Receive)
{
	const auto client = (Client*)parameter;
	if (!client) return NULL;

	LOG_DEBUG(CSTRING("[NET] - Receive thread has been started"));
	while (client->IsConnected())
		Kernel32::Sleep(client->DoReceive());

	// wait until thread has finished
	while (client && client->network.bLatency) Kernel32::Sleep(client->Isset(NET_OPT_FREQUENZ) ? client->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ);

	client->Clear();
	LOG_DEBUG(CSTRING("[NET] - Receive thread has been end"));
	return NULL;
}

bool Client::Isset(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return optionBitFlag & opt;
}

bool Client::Isset_SocketOpt(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return socketOptionBitFlag & opt;
}

bool Client::ChangeMode(const bool blocking)
{
	auto ret = true;
	u_long non_blocking = blocking ? 0 : 1;
	ret = NO_ERROR == ioctlsocket(GetSocket(), FIONBIO, &non_blocking);
	return ret;
}

char* Client::ResolveHostname(const char* name)
{
	WSADATA wsaData;
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[NET] - WSAStartup has been failed with error: %d"), res);
		return nullptr;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[NET] - Could not find a usable version of Winsock.dll"));
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
		LOG_ERROR(CSTRING("[NET] - Host look up has been failed with error %d"), dwRetval);
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
		case SOCK_STREAM:
			break;

		default:
			// skip
			continue;
		}

		switch (ptr->ai_protocol)
		{
		case IPPROTO_TCP:
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

bool Client::Connect(const char* Address, const u_short Port)
{
	if (IsConnected())
	{
		LOG_ERROR(CSTRING("[NET] - Can't connect to server, reason: already connected!"));
		return false;
	}

	WSADATA wsaData;
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[NET] - WSAStartup has been failed with error: %d"), res);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[NET] - Could not find a usable version of Winsock.dll"));
		WSACleanup();
		return false;
	}

	SetServerAddress(Address);
	SetServerPort(Port);

	auto v6 = AddrIsV6(GetServerAddress());
	auto v4 = AddrIsV4(GetServerAddress());
	if (!v6 && !v4)
	{
		LOG_ERROR(CSTRING("[NTP] - Address is neather IPV4 nor IPV6 Protocol"));
		WSACleanup();
		return false;
	}

	SetSocket(socket(v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
	if (GetSocket() == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[NTP] - Unable to create socket, error code: %d"), WSAGetLastError());
		WSACleanup();
		return false;
	}

	struct sockaddr* sockaddr = nullptr;
	int slen = NULL;
	if (v4)
	{
		struct sockaddr_in sockaddr4;
		memset((char*)&sockaddr4, 0, sizeof(sockaddr4));
		sockaddr4.sin_family = AF_INET;
		sockaddr4.sin_port = htons(GetServerPort());
		sockaddr4.sin_addr.S_un.S_addr = inet_addr(GetServerAddress());
		sockaddr = (struct sockaddr*)&sockaddr4;
		slen = static_cast<int>(sizeof(struct sockaddr_in));
	}

	if (v6)
	{
		struct sockaddr_in6 sockaddr6;
		memset((char*)&sockaddr6, 0, sizeof(sockaddr6));
		sockaddr6.sin6_family = AF_INET6;
		sockaddr6.sin6_port = htons(GetServerPort());
		res = inet_pton(AF_INET6, GetServerAddress(), &sockaddr6.sin6_addr);
		if (res != 1)
		{
			LOG_ERROR(CSTRING("[NTP]  - Failure on setting IPV6 Address with error code %d"), res);
			closesocket(GetSocket());
			WSACleanup();
			return false;
		}
		sockaddr = (struct sockaddr*)&sockaddr6;
		slen = static_cast<int>(sizeof(struct sockaddr_in6));
	}

	if (!sockaddr)
	{
		LOG_ERROR(CSTRING("[NTP]  - Socket is not being valid"));
		closesocket(GetSocket());
		WSACleanup();
		return false;
	}

	if (GetSocket() == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[Client] - socket failed with error: %ld"), WSAGetLastError());
		closesocket(GetSocket());
		WSACleanup();
		return false;
	}

	/* Connect to the server */
	if (connect(GetSocket(), sockaddr, slen) == SOCKET_ERROR)
	{
		closesocket(GetSocket());
		SetSocket(INVALID_SOCKET);

		LOG_ERROR(CSTRING("[Client] - failure on connecting to host: %s:%hu"), GetServerAddress(), GetServerPort());
		WSACleanup();
		return false;
	}

	// Set socket options
	for (const auto& entry : socketoption)
	{
		const auto res = _SetSocketOption(GetSocket(), entry);
		if (res < 0)
			LOG_ERROR(CSTRING("[Client] - Failure on settings socket option { 0x%ld : %i }"), entry.opt, GetLastError());
	}

	// clear the unused vector
	socketoption.clear();

	// successfully connected
	SetConnected(true);

	// Set Mode
	ChangeMode(Isset(NET_OPT_MODE_BLOCKING) ? GetOption<bool>(NET_OPT_MODE_BLOCKING) : NET_OPT_DEFAULT_MODE_BLOCKING);

	if (Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER)
		/* create RSA Key Pair */
		network.createNewRSAKeys(Isset(NET_OPT_CIPHER_RSA_SIZE) ? GetOption<size_t>(NET_OPT_CIPHER_RSA_SIZE) : NET_OPT_DEFAULT_RSA_SIZE);

	network.hCalcLatency = Timer::Create(CalcLatency, Isset(NET_OPT_INTERVAL_LATENCY) ? GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY, this);

	// spawn timer thread to sync clock with ntp - only effects having 2-step enabled
	if ((Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
		&& (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP))
		network.hSyncClockNTP = Timer::Create(NTPSyncClock, Isset(NET_OPT_NTP_SYNC_INTERVAL) ? GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL, this, true);

	// Create Loop-Receive Thread
	Thread::Create(Receive, this);

	// callback
	OnConnected();

	// if we use NTP execute the needed code
	if (Create2FASecret())
		LOG_DEBUG(CSTRING("[NET] - Successfully created 2FA-Hash"));

	return true;
}

bool Client::Disconnect()
{
	if (!IsConnected())
	{
		LOG_ERROR(CSTRING("[NET] - Can't disconnect from server, reason: not connected!"));
		return false;
	}

	// connection has been closed
	ConnectionClosed();

	// callback
	OnDisconnected();

	bAccomplished = TRUE;

	LOG_SUCCESS(CSTRING("[NET] - Disconnected from server"));
	return true;
}

void Client::Timeout()
{
	if (!IsConnected())
	{
		LOG_ERROR(CSTRING("[NET] - Can't disconnect from server, reason: not connected!"));
		return;
	}

	// connection has been closed
	ConnectionClosed();

	// callback
	OnTimeout();

	bAccomplished = TRUE;

	LOG_ERROR(CSTRING("[NET] - Connection has been closed, server did not answer anymore (TIMEOUT)"));
}

void Client::ConnectionClosed()
{
	if (GetSocket())
	{
		closesocket(GetSocket());
		SetSocket(INVALID_SOCKET);
	}

	network.latency = -1;
	network.bLatency = false;
	Timer::WaitSingleObjectStopped(network.hCalcLatency);
	Timer::WaitSingleObjectStopped(network.hSyncClockNTP);

	SetConnected(false);
}

void Client::Clear()
{
	if (IsConnected())
	{
		LOG_ERROR(CSTRING("[NET] - Can not clear Client while being connected!"));
		return;
	}

	WSACleanup();

	network.clear();
}

bool Client::DoNeedExit() const
{
	return NeedExit;
}

void Client::WaitUntilAccomplished()
{
	while (!bAccomplished)
		Kernel32::Sleep(FREQUENZ);
}

void Client::SetSocket(const SOCKET connectSocket)
{
	this->connectSocket = connectSocket;
}

SOCKET Client::GetSocket() const
{
	return connectSocket;
}

void Client::SetServerAddress(const char* ServerAddress)
{
	this->ServerAddress.free();
	const auto size = strlen(ServerAddress);
	this->ServerAddress = ALLOC<char>(size + 1);
	memcpy(this->ServerAddress.get(), ServerAddress, size);
	this->ServerAddress.get()[size] = '\0';
}

const char* Client::GetServerAddress() const
{
	return ServerAddress.get();
}

void Client::SetServerPort(const u_short ServerPort)
{
	this->ServerPort = ServerPort;
}

u_short Client::GetServerPort() const
{
	return ServerPort;
}

void Client::SetConnected(const bool connected)
{
	this->connected = connected;
}

bool Client::IsConnected() const
{
	return connected;
}

size_t Client::GetNextPackageSize() const
{
	return network.data_full_size;
}

size_t Client::GetReceivedPackageSize() const
{
	return network.data_size;
}

float Client::GetReceivedPackageSizeAsPerc() const
{
	// Avoid dividing zero with zero
	if (network.data_size <= 0)
		return 0.0f;

	if (network.data_full_size <= 0)
		return 0.0f;

	auto perc = static_cast<float>(network.data_size) / static_cast<float>(network.data_full_size) * 100;
	if (perc < 0.0f) // should never happen, just in case
		perc = 0.0f;
	else if (perc > 100.0f) // could happen, as we play around with the sizes later on
		perc = 100.0f;

	return perc;
}

void Client::Network::clear()
{
	recordingData = false;
	estabilished = false;
	clearData();
	deleteRSAKeys();

	FREE(fa2_secret);
	fa2_secret_len = NULL;
	curToken = NULL;
	lastToken = NULL;
	curTime = NULL;
	hSyncClockNTP = nullptr;
}

void Client::Network::AllocData(const size_t size)
{
	data = ALLOC<byte>(size + 1);
	memset(data.get(), NULL, size);
	data_size = 0;
	data_full_size = 0;
	data_offset = 0;
}

void Client::Network::clearData()
{
	data.free();
	data_size = 0;
	data_full_size = 0;
	data_offset = 0;
}

void Client::SetRecordingData(const bool status)
{
	network.recordingData = status;
}

void Client::Network::createNewRSAKeys(const size_t keySize)
{
	RSA = new NET_RSA();
	RSA->GenerateKeys(keySize, 3);
	RSAHandshake = false;
}

void Client::Network::deleteRSAKeys()
{
	delete RSA;
	RSAHandshake = false;
}

typeLatency Client::Network::getLatency() const
{
	return latency;
}

void Client::SingleSend(const char* data, size_t size, bool& bPreviousSentFailed)
{
	if (!GetSocket())
		return;

	if (bPreviousSentFailed)
		return;

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		char* ptr = (char*)data;
		for (size_t it = 0; it < size; ++it)
			ptr[it] = ptr[it] ^ network.curToken;
	}

	do
	{
		const auto res = send(GetSocket(), data, static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[NET] - Something bad happen... on Send"));
				Disconnect();
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);
}

void Client::SingleSend(BYTE*& data, size_t size, bool& bPreviousSentFailed)
{
	if (!GetSocket())
	{
		FREE(data);
		return;
	}

	if (bPreviousSentFailed)
	{
		FREE(data);
		return;
	}

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < size; ++it)
			data[it] = data[it] ^ network.curToken;
	}

	do
	{
		const auto res = send(GetSocket(), reinterpret_cast<const char*>(data), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[NET] - Something bad happen... on Send"));
				Disconnect();
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);

	FREE(data);
}

void Client::SingleSend(CPOINTER<BYTE>& data, size_t size, bool& bPreviousSentFailed)
{
	if (!GetSocket())
	{
		data.free();
		return;
	}

	if (bPreviousSentFailed)
	{
		data.free();
		return;
	}

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < size; ++it)
			data.get()[it] = data.get()[it] ^ network.curToken;
	}

	do
	{
		const auto res = send(GetSocket(), reinterpret_cast<const char*>(data.get()), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[NET] - Something bad happen... on Send"));
				Disconnect();
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);

	data.free();
}

/*
*							Visualisation of package structure in NET
*	------------------------------------------------------------------------------------------
*				CRYPTED VERSION					|		NON-CRYPTED VERSION
*	------------------------------------------------------------------------------------------
*	{KEYWORD}{SIZE}DATA
*	------------------------------------------------------------------------------------------
*	{BEGIN PACKAGE}								*		{BEGIN PACKAGE}
*		{PACKAGE SIZE}{...}						*			{PACKAGE SIZE}{...}
*			{KEY}{...}...						*						-
*			{IV}{...}...						*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...					*				{RAW DATA}{...}...
*			{DATA}{...}...						*				{DATA}{...}...
*	{END PACKAGE}								*		{END PACKAGE}
*
 */
void Client::DoSend(const int id, NET_PACKAGE pkg)
{
	if (!IsConnected())
		return;

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
		{
			network.lastToken = network.curToken;
			network.curToken = Net::Coding::FA2::generateToken(network.fa2_secret, network.fa2_secret_len, network.curTime, Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
		}
		else
		{
			network.lastToken = network.curToken;
			network.curToken = Net::Coding::FA2::generateToken(network.fa2_secret, network.fa2_secret_len, time(nullptr), Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
		}
	}

	rapidjson::Document JsonBuffer;
	JsonBuffer.SetObject();
	rapidjson::Value key(CSTRING("CONTENT"), JsonBuffer.GetAllocator());
	JsonBuffer.AddMember(key, PKG.GetPackage(), JsonBuffer.GetAllocator());
	rapidjson::Value keyID(CSTRING("ID"), JsonBuffer.GetAllocator());

	rapidjson::Value idValue;
	idValue.SetInt(id);
	JsonBuffer.AddMember(keyID, idValue, JsonBuffer.GetAllocator());

	/* tmp buffer, later we cast to PBYTE */
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	JsonBuffer.Accept(writer);

	size_t combinedSize = NULL;

	/* Crypt */
	if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && network.RSAHandshake)
	{
		NET_AES aes;

		/* Generate new AES Keypair */
		size_t aesKeySize = Isset(NET_OPT_CIPHER_AES_SIZE) ? GetOption<size_t>(NET_OPT_CIPHER_AES_SIZE) : NET_OPT_DEFAULT_AES_SIZE;
		CPOINTER<BYTE> Key(ALLOC<BYTE>(aesKeySize + 1));
		Random::GetRandStringNew(Key.reference().get(), aesKeySize);
		Key.get()[aesKeySize] = '\0';

		CPOINTER<BYTE> IV(ALLOC<BYTE>(CryptoPP::AES::BLOCKSIZE + 1));
		Random::GetRandStringNew(IV.reference().get(), CryptoPP::AES::BLOCKSIZE);
		IV.get()[CryptoPP::AES::BLOCKSIZE] = '\0';

		if (!aes.Init(reinterpret_cast<char*>(Key.get()), reinterpret_cast<char*>(IV.get())))
		{
			Key.free();
			IV.free();

			LOG_ERROR(CSTRING("[NET] - Failed to Init AES [0]"));
			Disconnect();
			return;
		}

		if (!network.RSA)
		{
			Key.free();
			IV.free();

			LOG_ERROR(CSTRING("[NET] - RSA Object has no instance"));
			Disconnect();
			return;
		}

		/* Encrypt AES Keypair using RSA */
		if (!network.RSA->encryptBase64(Key.reference().get(), aesKeySize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("[NET] - Failed Key to encrypt and encode to base64"));
			Disconnect();
			return;
		}

		size_t IVSize = CryptoPP::AES::BLOCKSIZE;
		if (!network.RSA->encryptBase64(IV.reference().get(), IVSize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("[NET] - Failed IV to encrypt and encode to base64"));
			Disconnect();
			return;
		}

		/* Crypt Buffer using AES and Encode to Base64 */
		auto dataBufferSize = buffer.GetSize();
		CPOINTER<BYTE> dataBuffer(ALLOC<BYTE>(dataBufferSize + 1));
		memcpy(dataBuffer.get(), buffer.GetString(), dataBufferSize);
		buffer.Flush();
		dataBuffer.get()[dataBufferSize] = '\0';
		aes.encrypt(dataBuffer.get(), dataBufferSize);

		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			CompressData(dataBuffer.reference().get(), dataBufferSize);

		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto& data : rawData)
				aes.encrypt(data.value(), data.size());
		}

		combinedSize = dataBufferSize + strlen(NET_PACKAGE_HEADER) + strlen(NET_PACKAGE_SIZE) + strlen(NET_DATA) + strlen(NET_PACKAGE_FOOTER) + strlen(NET_AES_KEY) + strlen(NET_AES_IV) + aesKeySize + IVSize + 8;

		// Append Raw data package size
		if (PKG.HasRawData())
		{
			if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.value(), data.size());
			}

			combinedSize += PKG.GetRawDataFullSize();
		}

		std::string dataSizeStr;
		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
		{
			dataSizeStr = std::to_string(dataBufferSize);
			combinedSize += dataSizeStr.length();
		}
		else
		{
			dataSizeStr = std::to_string(buffer.GetSize());
			combinedSize += dataSizeStr.length();
		}

		const auto KeySizeStr = std::to_string(aesKeySize);
		combinedSize += KeySizeStr.length();

		const auto IVSizeStr = std::to_string(IVSize);
		combinedSize += IVSizeStr.length();

		const auto EntirePackageSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

		auto bPreviousSentFailed = false;

		/* Append Package Header */
		SingleSend(NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER), bPreviousSentFailed);

		// Append Package Size Syntax
		SingleSend(NET_PACKAGE_SIZE, strlen(NET_PACKAGE_SIZE), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		/* Append Package Key */
		SingleSend(NET_AES_KEY, strlen(NET_AES_KEY), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(KeySizeStr.data(), KeySizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(Key, aesKeySize, bPreviousSentFailed);

		/* Append Package IV */
		SingleSend(NET_AES_IV, strlen(NET_AES_IV), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(IVSizeStr.data(), IVSizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(IV, IVSize, bPreviousSentFailed);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(data.key(), data.keylength() + 1, bPreviousSentFailed);

				// Append Raw Data
				SingleSend(NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(data.value(), data.size(), bPreviousSentFailed);
				PKG.DoNotDestruct();
			}
		}

		SingleSend(NET_DATA, strlen(NET_DATA), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed);

		/* Append Package Footer */
		SingleSend(NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER), bPreviousSentFailed);
	}
	else
	{
		CPOINTER<BYTE> dataBuffer;
		size_t dataBufferSize = NULL;
		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
		{
			dataBufferSize = buffer.GetSize();
			dataBuffer = ALLOC<BYTE>(dataBufferSize + 1);
			memcpy(dataBuffer.get(), buffer.GetString(), dataBufferSize);
			buffer.Flush();
			dataBuffer.get()[dataBufferSize] = '\0';

			CompressData(dataBuffer.reference().get(), dataBufferSize);

			combinedSize = dataBufferSize + strlen(NET_PACKAGE_HEADER) + strlen(NET_PACKAGE_SIZE) + strlen(NET_DATA) + strlen(NET_PACKAGE_FOOTER) + 4;
		}
		else
			combinedSize = buffer.GetSize() + strlen(NET_PACKAGE_HEADER) + strlen(NET_PACKAGE_SIZE) + strlen(NET_DATA) + strlen(NET_PACKAGE_FOOTER) + 4;

		// Append Raw data package size
		if (PKG.HasRawData())
		{
			if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.value(), data.size());
			}

			combinedSize += PKG.GetRawDataFullSize();
		}

		std::string dataSizeStr;
		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
		{
			dataSizeStr = std::to_string(dataBufferSize);
			combinedSize += dataSizeStr.length();
		}
		else
		{
			dataSizeStr = std::to_string(buffer.GetSize());
			combinedSize += dataSizeStr.length();
		}

		const auto EntirePackageSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

		auto bPreviousSentFailed = false;

		/* Append Package Header */
		SingleSend(NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER), bPreviousSentFailed);

		// Append Package Size Syntax
		SingleSend(NET_PACKAGE_SIZE, strlen(NET_PACKAGE_SIZE), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(data.key(), data.keylength() + 1, bPreviousSentFailed);

				// Append Raw Data
				SingleSend(NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(data.value(), data.size(), bPreviousSentFailed);
				PKG.DoNotDestruct();
			}
		}

		SingleSend(NET_DATA, strlen(NET_DATA), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_OPEN, strlen(NET_PACKAGE_BRACKET_OPEN), bPreviousSentFailed);
		SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
		SingleSend(NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed);
		else
			SingleSend(buffer.GetString(), buffer.GetSize(), bPreviousSentFailed);

		/* Append Package Footer */
		SingleSend(NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER), bPreviousSentFailed);
	}
}

/*
*							Visualisation of package structure in NET
*	------------------------------------------------------------------------------------------
*				CRYPTED VERSION					|		NON-CRYPTED VERSION
*	------------------------------------------------------------------------------------------
*	{KEYWORD}{SIZE}DATA
*	------------------------------------------------------------------------------------------
*	{BEGIN PACKAGE}								*		{BEGIN PACKAGE}
*		{PACKAGE SIZE}{...}						*			{PACKAGE SIZE}{...}
*			{KEY}{...}...						*						-
*			{IV}{...}...						*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...					*				{RAW DATA}{...}...
*			{DATA}{...}...						*				{DATA}{...}...
*	{END PACKAGE}								*		{END PACKAGE}
*
 */
DWORD Client::DoReceive()
{
	if (!IsConnected())
		return FREQUENZ;

	const auto data_size = recv(GetSocket(), reinterpret_cast<char*>(network.dataReceive), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - A successful WSAStartup() call must occur before using this function"));
			Disconnect();
			return FREQUENZ;

		case WSAENETDOWN:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The network subsystem has failed"));
			Disconnect();
			return FREQUENZ;

		case WSAEFAULT:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The buf parameter is not completely contained in a valid part of the user address space"));
			Disconnect();
			return FREQUENZ;

		case WSAENOTCONN:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The socket is not connected"));
			Disconnect();
			return FREQUENZ;

		case WSAEINTR:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The (blocking) call was canceled through WSACancelBlockingCall()"));
			Disconnect();
			return FREQUENZ;

		case WSAEINPROGRESS:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"));
			Disconnect();
			return FREQUENZ;

		case WSAENETRESET:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"));
			Disconnect();
			return FREQUENZ;

		case WSAENOTSOCK:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The descriptor is not a socket"));
			Disconnect();
			return FREQUENZ;

		case WSAEOPNOTSUPP:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"));
			Disconnect();
			return FREQUENZ;

		case WSAESHUTDOWN:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"));
			Disconnect();
			return FREQUENZ;

		case WSAEWOULDBLOCK:
			ProcessPackages();
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			return FREQUENZ;

		case WSAEMSGSIZE:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The message was too large to fit into the specified buffer and was truncated"));
			Disconnect();
			return FREQUENZ;

		case WSAEINVAL:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"));
			Disconnect();
			return FREQUENZ;

		case WSAECONNABORTED:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
			Timeout();
			return FREQUENZ;

		case WSAETIMEDOUT:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The connection has been dropped because of a network failure or because the peer system failed to respond"));
			Timeout();
			return FREQUENZ;

		case WSAECONNRESET:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"));
			Timeout();
			return FREQUENZ;

		default:
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[NET] - Something bad happen..."));
			Disconnect();
			return FREQUENZ;
		}
	}
	if (data_size == 0)
	{
		memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
		LOG_PEER(CSTRING("[NET] - Connection has been gracefully closed"));
		Disconnect();
		return FREQUENZ;
	}

	if (!network.data.valid())
	{
		network.AllocData(data_size);
		memcpy(network.data.get(), network.dataReceive, data_size);
		network.data.get()[data_size] = '\0';
		network.data_size = data_size;
	}
	else
	{
		if (network.data_full_size > 0
			&& network.data_size + data_size < network.data_full_size)
		{
			memcpy(&network.data.get()[network.data_size], network.dataReceive, data_size);
			network.data_size += data_size;
		}
		else
		{
			/* store incomming */
			const auto newBuffer = ALLOC<BYTE>(network.data_size + data_size + 1);
			memcpy(newBuffer, network.data.get(), network.data_size);
			memcpy(&newBuffer[network.data_size], network.dataReceive, data_size);
			newBuffer[network.data_size + data_size] = '\0';
			network.data = newBuffer; // pointer swap
			network.data_size += data_size;
		}
	}

	memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
	ProcessPackages();
	return NULL;
}

void Client::ProcessPackages()
{
	// check valid data size
	if (!network.data_size
		|| network.data_size == INVALID_SIZE)
		return;

	auto use_old_token = true;
	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		// shift the first bytes to check if we are using the correct token - using old token
		for (size_t it = 0; it < strlen(NET_PACKAGE_HEADER); ++it)
			network.data.get()[it] = network.data.get()[it] ^ network.lastToken;

		if (memcmp(&network.data.get()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
		{
			// shift back
			for (size_t it = 0; it < strlen(NET_PACKAGE_HEADER); ++it)
				network.data.get()[it] = network.data.get()[it] ^ network.lastToken;

			if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
			{
				network.lastToken = network.curToken;
				network.curToken = Net::Coding::FA2::generateToken(network.fa2_secret, network.fa2_secret_len, network.curTime, Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
			}
			else
			{
				network.lastToken = network.curToken;
				network.curToken = Net::Coding::FA2::generateToken(network.fa2_secret, network.fa2_secret_len, time(nullptr), Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
			}

			// shift the first bytes to check if we are using the correct token - using new token
			for (size_t it = 0; it < strlen(NET_PACKAGE_HEADER); ++it)
				network.data.get()[it] = network.data.get()[it] ^ network.curToken;

			// [PROTOCOL] - check header is actually valid
			if (memcmp(&network.data.get()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
			{
				LOG_ERROR(CSTRING("[NET] - Frame has no valid header... dropping frame"));
				network.clearData();
				Disconnect();
				return;
			}

			use_old_token = false;
		}
	}
	else
	{
		// [PROTOCOL] - check header is actually valid
		if (memcmp(&network.data.get()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
		{
			LOG_ERROR(CSTRING("[NET] - Frame has no valid header... dropping frame"));
			network.clearData();
			Disconnect();
			return;
		}
	}

	// [PROTOCOL] - read data full size from header
	const auto offset = static_cast<int>(strlen(NET_PACKAGE_HEADER)) + static_cast<int>(strlen(NET_PACKAGE_SIZE)); // skip header tags
	for (size_t i = offset; i < network.data_size; ++i)
	{
		// shift the byte
		if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
			network.data.get()[i] = network.data.get()[i] ^ (use_old_token ? network.lastToken : network.curToken);

		// iterate until we have found the end tag
		if (!memcmp(&network.data.get()[i], NET_PACKAGE_BRACKET_CLOSE, 1))
		{
			const auto bDoPreAlloc = network.data_full_size == 0 ? true : false;
			network.data_offset = i;
			const auto size = i - offset - 1;
			CPOINTER<BYTE> sizestr(ALLOC<BYTE>(size + 1));
			memcpy(sizestr.get(), &network.data.get()[offset + 1], size);
			sizestr.get()[size] = '\0';
			network.data_full_size = strtoull(reinterpret_cast<const char*>(sizestr.get()), nullptr, 10);
			sizestr.free();

			// awaiting more bytes
			if (bDoPreAlloc &&
				network.data_full_size > network.data_size)
			{
				// pre-allocate enough space
				const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
				memcpy(newBuffer, network.data.get(), network.data_size);
				newBuffer[network.data_full_size] = '\0';
				network.data = newBuffer; // pointer swap

				// shift all the way back
				if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
				{
					for (size_t it = 0; it < i; ++it)
						network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
				}

				return;
			}

			// shift all the way back
			if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
			{
				for (size_t it = 0; it < i; ++it)
					network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
			}

			break;
		}
	}

	// keep going until we have received the entire package
	if (network.data_size < network.data_full_size) return;

	// shift only as much as required
	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < network.data_full_size; ++it)
			network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
	}

	// [PROTOCOL] - check footer is actually valid
	if (memcmp(&network.data.get()[network.data_full_size - strlen(NET_PACKAGE_FOOTER)], NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER)) != 0)
	{
		LOG_ERROR(CSTRING("[NET] - Frame has no valid footer... dropping frame"));
		network.clearData();
		Disconnect();
		return;
	}

	// Execute the package
	ExecutePackage();

	// re-alloc buffer
	const auto leftSize = static_cast<int>(network.data_size - network.data_full_size) > 0 ? network.data_size - network.data_full_size : INVALID_SIZE;
	if (leftSize != INVALID_SIZE
		&& leftSize > 0)
	{
		const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
		memcpy(leftBuffer, &network.data.get()[network.data_full_size], leftSize);
		leftBuffer[leftSize] = '\0';
		network.clearData();
		network.data = leftBuffer; // swap pointer
		network.data_size = leftSize;
		return;
	}

	network.clearData();
}

void Client::ExecutePackage()
{
	CPOINTER<BYTE> data;
	std::vector<Package_RawData_t> rawData;

	/* Crypt */
	if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && network.RSAHandshake)
	{
		auto offset = network.data_offset + 1;

		CPOINTER<BYTE> AESKey;
		size_t AESKeySize;

		// look for key tag
		if (!memcmp(&network.data.get()[offset], NET_AES_KEY, strlen(NET_AES_KEY)))
		{
			offset += strlen(NET_AES_KEY);

			// read size
			for (auto y = offset; y < network.data_size; ++y)
			{
				if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESKeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESKey = ALLOC<BYTE>(AESKeySize + 1);
			memcpy(AESKey.get(), &network.data.get()[offset], AESKeySize);
			AESKey.get()[AESKeySize] = '\0';

			offset += AESKeySize;
		}

		CPOINTER<BYTE> AESIV;
		size_t AESIVSize;

		// look for iv tag
		if (!memcmp(&network.data.get()[offset], NET_AES_IV, strlen(NET_AES_IV)))
		{
			offset += strlen(NET_AES_IV);

			// read size
			for (auto y = offset; y < network.data_size; ++y)
			{
				if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESIVSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESIV = ALLOC<BYTE>(AESIVSize + 1);
			memcpy(AESIV.get(), &network.data.get()[offset], AESIVSize);
			AESIV.get()[AESIVSize] = '\0';

			offset += AESIVSize;
		}

		if (!network.RSA)
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Failure on initializing RSA"));
			return;
		}

		if (!network.RSA->decryptBase64(AESKey.reference().get(), AESKeySize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-Key & RSA and Base64"));
			return;
		}

		if (!network.RSA->decryptBase64(AESIV.reference().get(), AESIVSize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-IV & RSA and Base64"));
			return;
		}

		NET_AES aes;
		if (!aes.Init(reinterpret_cast<char*>(AESKey.get()), reinterpret_cast<char*>(AESIV.get())))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Initializing AES failure"));
			return;
		}

		AESKey.free();
		AESIV.free();

		do
		{
			// look for raw data tag
			if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
			{
				offset += strlen(NET_RAW_DATA_KEY);

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &network.data.get()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
				{
					offset += strlen(NET_RAW_DATA);

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < network.data_size; ++y)
						{
							if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &network.data.get()[offset], packageSize };

					if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						DecompressData(entry.value(), entry.size());

					/* decrypt aes */
					if (!aes.decrypt(entry.value(), entry.size()))
					{
						LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));
						return;
					}

					rawData.emplace_back(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&network.data.get()[offset], NET_DATA, strlen(NET_DATA)))
			{
				offset += strlen(NET_DATA);

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}
				}

				// read the data
				data = ALLOC<BYTE>(packageSize + 1);
				memcpy(data.get(), &network.data.get()[offset], packageSize);
				data.get()[packageSize] = '\0';

				offset += packageSize;

				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					DecompressData(data.reference().get(), packageSize);

				/* decrypt aes */
				if (!aes.decrypt(data.get(), packageSize))
				{
					LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));
					data.free();
					return;
				}
			}

			// we have reached the end of reading
			if (offset + strlen(NET_PACKAGE_FOOTER) == network.data_full_size)
				break;

		} while (true);
	}
	else
	{
		auto offset = network.data_offset + 1;

		do
		{
			// look for raw data tag
			if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
			{
				offset += strlen(NET_RAW_DATA_KEY);

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &network.data.get()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
				{
					offset += strlen(NET_RAW_DATA);

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < network.data_size; ++y)
						{
							if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &network.data.get()[offset], packageSize };

					if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						DecompressData(entry.value(), entry.size());

					rawData.emplace_back(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&network.data.get()[offset], NET_DATA, strlen(NET_DATA)))
			{
				offset += strlen(NET_DATA);

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}
				}

				// read the data
				data = ALLOC<BYTE>(packageSize + 1);
				memcpy(data.get(), &network.data.get()[offset], packageSize);
				data.get()[packageSize] = '\0';

				offset += packageSize;

				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					DecompressData(data.reference().get(), packageSize);
			}

			// we have reached the end of reading
			if (offset + strlen(NET_PACKAGE_FOOTER) == network.data_full_size)
				break;

		} while (true);
	}

	if (!data.valid())
	{
		LOG_PEER(CSTRING("[NET] - JSON data is not valid"));
		return;
	}

	Package PKG;
	PKG.Parse(reinterpret_cast<char*>(data.get()));
	if (!PKG.GetPackage().HasMember(CSTRING("ID")))
	{
		LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));
		data.free();
		return;
	}

	const auto id = PKG.GetPackage().FindMember(CSTRING("ID"))->value.GetInt();
	if (id < 0)
	{
		LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));
		data.free();
		return;
	}

	if (!PKG.GetPackage().HasMember(CSTRING("CONTENT")))
	{
		LOG_PEER(CSTRING("[NET] - Frame is empty"));
		data.free();
		return;
	}

	Package Content;
	Content.DoNotDestruct();

	if (!PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.IsNull())
		Content.SetPackage(PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.GetObject());

	// set raw data
	if (!rawData.empty())
		Content.SetRawData(rawData);

	if (!CheckDataN(id, Content))
		if (!CheckData(id, Content))
		{
			LOG_PEER(CSTRING("[NET] - Frame is not defined"));
			Disconnect();
		}

	data.free();
}

void Client::CompressData(BYTE*& data, size_t& size)
{
	/* Compression */
	if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
	{
#ifdef DEBUG
		const auto PrevSize = size;
#endif
		NET_ZLIB::Compress(data, size);
#ifdef DEBUG
		LOG_DEBUG(CSTRING("[NET] - Compressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

bool Client::Create2FASecret()
{
	if (!(Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA))
		return false;

	network.curTime = time(nullptr);
	if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
	{
		const auto time = Net::Protocol::NTP::Exec(Isset(NET_OPT_NTP_HOST) ? GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
			Isset(NET_OPT_NTP_PORT) ? GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

		if (!time.valid())
		{
			LOG_ERROR(CSTRING("[NET] - critical failure on calling NTP host"));
			return false;
		}

		network.curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
	}

	tm tm;
	gmtime_s(&tm, &network.curTime);
	tm.tm_hour = Net::Util::roundUp(tm.tm_hour, 10);
	tm.tm_min = Net::Util::roundUp(tm.tm_min, 10);
	tm.tm_sec = 0;
	const auto txTm = mktime(&tm);

	const auto strTime = ctime(&txTm);
	network.fa2_secret_len = strlen(strTime);

	FREE(network.fa2_secret);
	network.fa2_secret = ALLOC<byte>(network.fa2_secret_len + 1);
	memcpy(network.fa2_secret, strTime, network.fa2_secret_len);
	network.fa2_secret[network.fa2_secret_len] = '\0';
	Net::Coding::Base32::base32_encode(network.fa2_secret, network.fa2_secret_len);

	return true;
}

void Client::DecompressData(BYTE*& data, size_t& size)
{
	/* Compression */
	if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
	{
		auto copy = ALLOC<BYTE>(size + 1);
		memcpy(copy, data, size);
		copy[size] = '\0';

#ifdef DEBUG
		const auto PrevSize = size;
#endif
		NET_ZLIB::Decompress(copy, size);

		data = copy; // swap pointer;
#ifdef DEBUG
		LOG_DEBUG(CSTRING("[NET] - Decompressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

NET_CLIENT_BEGIN_DATA_PACKAGE_NATIVE(Client)
NET_CLIENT_DEFINE_PACKAGE(RSAHandshake, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake)
NET_CLIENT_DEFINE_PACKAGE(VersionPackage, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage)
NET_CLIENT_DEFINE_PACKAGE(EstabilishConnectionPackage, NET_NATIVE_PACKAGE_ID::PKG_EstabilishPackage)
NET_CLIENT_DEFINE_PACKAGE(ClosePackage, NET_NATIVE_PACKAGE_ID::PKG_ClosePackage)
NET_CLIENT_END_DATA_PACKAGE

NET_BEGIN_FNC_PKG(Client, RSAHandshake)
if (!(Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER))
{
	LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, cipher option is been disabled, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}
if (network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, client has already performed a handshake, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}

NET_JOIN_PACKAGE(pkg, pkgRel);

const auto publicKey = pkgRel.String(CSTRING("PublicKey"));
if (!publicKey.valid())
{
	LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, received public key is not valid, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}

// send our generated Public Key to the Server
const auto PublicKeyRef = network.RSA->PublicKey();
pkgRel.Rewrite<const char*>(CSTRING("PublicKey"), PublicKeyRef.get());
NET_SEND(NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake, pkgRel);

network.RSA->SetPublicKey((char*)publicKey.value());

// from now we use the Cryption, synced with Server
network.RSAHandshake = true;
NET_END_FNC_PKG

NET_BEGIN_FNC_PKG(Client, VersionPackage)
if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && !network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[NET][%s] - received a version frame, client has not performed a handshake yet, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[NET][%s] - received a version frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}

NET_JOIN_PACKAGE(pkg, pkgRel);

const auto Key = Version::Key().data(); // otherwise we memleak

pkgRel.Append<int>(CSTRING("MajorVersion"), Version::Major());
pkgRel.Append<int>(CSTRING("MinorVersion"), Version::Minor());
pkgRel.Append<int>(CSTRING("Revision"), Version::Revision());
pkgRel.Append<const char*>(CSTRING("Key"), Key.data());
NET_SEND(NET_NATIVE_PACKAGE_ID::PKG_VersionPackage, pkgRel);
NET_END_FNC_PKG

NET_BEGIN_FNC_PKG(Client, EstabilishConnectionPackage)
if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && !network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[NET][%s] - received an estabilishing frame, client has not performed a handshake yet, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[NET][%s] - received an estabilishing frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
	Disconnect();
	return;
}

network.estabilished = true;

// Callback
// connection has been estabilished, now call entry function
OnConnectionEstabilished();
NET_END_FNC_PKG

NET_BEGIN_FNC_PKG(Client, ClosePackage)
// connection has been closed
ConnectionClosed();

LOG_SUCCESS(CSTRING("[NET] - Connection has been closed by the Server"));

const auto code = PKG.Int(CSTRING("code"));
if (!code.valid())
{
	// Callback
	OnForcedDisconnect(-1);
	return;
}

// callback Different Version
if (code.value() == NET_ERROR_CODE::NET_ERR_Versionmismatch)
OnVersionMismatch();

// Callback
OnForcedDisconnect(code.value());
NET_END_FNC_PKG
NET_NAMESPACE_END
NET_NAMESPACE_END