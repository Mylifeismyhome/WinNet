#include <NetServer/Server.h>
#include <Net/Import/Kernel32.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Server)
IPRef::IPRef(PCSTR const pointer)
{
	this->pointer = (char*)pointer;
}

IPRef::~IPRef()
{
	FREE(pointer);
}

PCSTR IPRef::get() const
{
	return pointer;
}

Server::Server()
{
	Net::Codes::NetLoadErrorCodes();
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);
	SetRunning(false);
}

Server::~Server()
{
}

bool Server::Isset(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return optionBitFlag & opt;
}

bool Server::Isset_SocketOpt(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return socketOptionBitFlag & opt;
}

#pragma region Network Structure
void Server::network_t::setData(byte* pointer)
{
	deallocData();
	_data = pointer;
}

void Server::network_t::allocData(const size_t size)
{
	clear();
	_data = ALLOC<byte>(size + 1);
	memset(getData(), NULL, size * sizeof(byte));
	getData()[size] = '\0';

	setDataSize(size);
}

void Server::network_t::deallocData()
{
	_data.free();
}

byte* Server::network_t::getData() const
{
	return _data.get();
}

void Server::network_t::reset()
{
	memset(_dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
}

void Server::network_t::clear()
{
	deallocData();
	_data_size = NULL;
	_data_full_size = NULL;
	_data_offset = NULL;
}

void Server::network_t::setDataSize(const size_t size)
{
	_data_size = size;
}

size_t Server::network_t::getDataSize() const
{
	return _data_size;
}

void Server::network_t::setDataFullSize(const size_t size)
{
	_data_full_size = size;
}

size_t Server::network_t::getDataFullSize() const
{
	return _data_full_size;
}

void Server::network_t::SetDataOffset(const size_t offset)
{
	_data_offset = offset;
}

size_t Server::network_t::getDataOffset() const
{
	return _data_offset;
}

bool Server::network_t::dataValid() const
{
	return _data.valid();
}

byte* Server::network_t::getDataReceive()
{
	return _dataReceive;
}
#pragma endregion

#pragma region Cryption Structure
void Server::cryption_t::createKeyPair(const size_t size)
{
	RSA = new NET_RSA();
	RSA->GenerateKeys(size, 3);
	setHandshakeStatus(false);
}

void Server::cryption_t::deleteKeyPair()
{
	delete RSA;
	setHandshakeStatus(false);
}

void Server::cryption_t::setHandshakeStatus(const bool status)
{
	RSAHandshake = status;
}

bool Server::cryption_t::getHandshakeStatus() const
{
	return RSAHandshake;
}
#pragma endregion

void Server::IncreasePeersCounter()
{
	++_CounterPeersTable;
}

void Server::DecreasePeersCounter()
{
	--_CounterPeersTable;

	if (_CounterPeersTable == INVALID_SIZE)
		_CounterPeersTable = NULL;
}

struct	 CalcLatency_t
{
	Server* server;
	Server::NET_PEER peer;
};

NET_TIMER(CalcLatency)
{
	const auto info = (CalcLatency_t*)param;
	if (!info) NET_STOP_TIMER;

	const auto server = info->server;
	const auto peer = info->peer;

	peer->bLatency = true;
	peer->latency = Net::Protocol::ICMP::Exec(peer->IPAddr().get());
	peer->bLatency = false;

	Timer::SetTime(peer->hCalcLatency, server->Isset(NET_OPT_INTERVAL_LATENCY) ? server->GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY);
	NET_CONTINUE_TIMER;
}

struct NTPSyncClock_t
{
	Server* server;
	Server::NET_PEER peer;
};

NET_TIMER(NTPSyncClock)
{
	const auto info = (NTPSyncClock_t*)param;
	if (!info) NET_STOP_TIMER;

	const auto server = info->server;
	const auto peer = info->peer;

	const auto time = Net::Protocol::NTP::Exec(server->Isset(NET_OPT_NTP_HOST) ? server->GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
		server->Isset(NET_OPT_NTP_PORT) ? server->GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

	if (!time.valid())
	{
		LOG_ERROR(CSTRING("[%s] - critical failure on calling NTP host"), SERVERNAME(server));
		NET_CONTINUE_TIMER;
	}

	peer->curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

	Timer::SetTime(peer->hSyncClockNTP, server->Isset(NET_OPT_NTP_SYNC_INTERVAL) ? server->GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL);
	NET_CONTINUE_TIMER;
}

Server::NET_PEER Server::CreatePeer(const sockaddr_in client_addr, const SOCKET socket)
{
	// UniqueID is equal to socket, since socket is already an unique ID
	const auto peer = new NET_IPEER();
	peer->UniqueID = socket;
	peer->pSocket = socket;
	peer->client_addr = client_addr;

	/* Set Read Timeout */
	timeval tv = {};
	tv.tv_sec = Isset(NET_OPT_TIMEOUT_TCP_READ) ? GetOption<long>(NET_OPT_TIMEOUT_TCP_READ) : NET_OPT_DEFAULT_TIMEOUT_TCP_READ;
	tv.tv_usec = 0;
	setsockopt(peer->pSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	const auto _CalcLatency = new CalcLatency_t();
	_CalcLatency->server = this;
	_CalcLatency->peer = peer;
	peer->hCalcLatency = Timer::Create(CalcLatency, Isset(NET_OPT_INTERVAL_LATENCY) ? GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY, _CalcLatency, true);

	// spawn timer thread to sync clock with ntp - only effects having 2-step enabled
	if ((Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
		&& (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP))
	{
		const auto _NTPSyncClock_t = new NTPSyncClock_t();
		_NTPSyncClock_t->server = this;
		_NTPSyncClock_t->peer = peer;
		peer->hSyncClockNTP = Timer::Create(NTPSyncClock, Isset(NET_OPT_NTP_SYNC_INTERVAL) ? GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL, _NTPSyncClock_t, true);
	}

	IncreasePeersCounter();

	// callback
	OnPeerConnect(peer);

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): connected"), SERVERNAME(this), peer->IPAddr().get());

	if (Create2FASecret(peer))
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): successfully created 2FA-Hash"), SERVERNAME(this), peer->IPAddr().get());

	return peer;
}

bool Server::ErasePeer(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return false;
	);

	NET_PEER_WAIT_LOCK(peer);
	peer->lock();

	if (peer->bHasBeenErased)
		return false;

	if (!peer->isAsync)
	{
		// close endpoint
		SOCKET_VALID(peer->pSocket)
		{
			closesocket(peer->pSocket);
			peer->pSocket = INVALID_SOCKET;
		}

		if (peer->hCalcLatency)
		{
			// stop latency interval
			Timer::WaitSingleObjectStopped(peer->hCalcLatency);
			peer->hCalcLatency = nullptr;
		}

		if (peer->hSyncClockNTP)
		{
			// stop latency interval
			Timer::WaitSingleObjectStopped(peer->hSyncClockNTP);
			peer->hSyncClockNTP = nullptr;
		}

		// callback
		OnPeerDisconnect(peer);

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): disconnected"), SERVERNAME(this), peer->IPAddr().get());

		peer->clear();

		DecreasePeersCounter();

		peer->bHasBeenErased = true;

		peer->unlock();
		return true;
	}

	// close endpoint
	SOCKET_VALID(peer->pSocket)
	{
		closesocket(peer->pSocket);
		peer->pSocket = INVALID_SOCKET;
	}

	if (peer->hCalcLatency)
	{
		// stop latency interval
		Timer::WaitSingleObjectStopped(peer->hCalcLatency);
		peer->hCalcLatency = nullptr;
	}

	if (peer->hSyncClockNTP)
	{
		// stop latency interval
		Timer::WaitSingleObjectStopped(peer->hSyncClockNTP);
		peer->hSyncClockNTP = nullptr;
	}

	peer->unlock();
	return true;
}

size_t Server::GetNextPackageSize(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return NULL;
	);

	return peer->network.getDataFullSize();
}

size_t Server::GetReceivedPackageSize(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return NULL;
	);

	return peer->network.getDataSize();
}

float Server::GetReceivedPackageSizeAsPerc(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return 0.0f;
	);

	// Avoid dividing zero with zero
	if (peer->network.getDataSize() <= 0)
		return 0.0f;

	if (peer->network.getDataFullSize() <= 0)
		return 0;

	auto perc = static_cast<float>(peer->network.getDataSize()) / static_cast<float>(peer->network.getDataFullSize()) * 100;
	if (perc < 0.0f) // should never happen, just in case
		perc = 0.0f;
	else if (perc > 100.0f) // could happen, as we play around with the sizes later on
		perc = 100.0f;

	return perc;
}

void Server::NET_IPEER::clear()
{
	UniqueID = INVALID_UID;
	pSocket = INVALID_SOCKET;
	client_addr = sockaddr_in();
	estabilished = false;
	isAsync = false;
	NetVersionMatched = false;
	bLatency = false;
	latency = -1;
	hCalcLatency = nullptr;

	network.clear();
	network.reset();

	cryption.deleteKeyPair();

	FREE(fa2_secret);
	fa2_secret_len = NULL;
	curToken = NULL;
	lastToken = NULL;
	curTime = NULL;
	hSyncClockNTP = nullptr;
}

void Server::NET_IPEER::setAsync(const bool status)
{
	isAsync = status;
}

typeLatency Server::NET_IPEER::getLatency() const
{
	return latency;
}

IPRef Server::NET_IPEER::IPAddr() const
{
	const auto buf = ALLOC<char>(INET_ADDRSTRLEN);
#ifdef VS13
	return IPRef(inet_ntop(AF_INET, (PVOID)&client_addr.sin_addr, buf, INET_ADDRSTRLEN));
#else
	return IPRef(inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN));
#endif
}

void Server::NET_IPEER::lock()
{
	bQueueLock = true;
}

void Server::NET_IPEER::unlock()
{
	bQueueLock = false;
}

void Server::DisconnectPeer(NET_PEER peer, const int code, const bool skipNotify)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (!skipNotify)
	{
		Package PKG;
		PKG.Append(CSTRING("code"), code);
		NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_ClosePackage, pkg);
	}

	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): has been disconnected, reason: %s"), SERVERNAME(this), peer->IPAddr().get(), Net::Codes::NetGetErrorMessage(code));

	// now after we have sent him the reason, close connection
	ErasePeer(peer);
}

void Server::SetListenSocket(const SOCKET ListenSocket)
{
	this->ListenSocket = ListenSocket;
}

void Server::SetAcceptSocket(const SOCKET AcceptSocket)
{
	this->AcceptSocket = AcceptSocket;
}

void Server::SetRunning(const bool bRunning)
{
	this->bRunning = bRunning;
}

SOCKET Server::GetListenSocket() const
{
	return ListenSocket;
}

SOCKET Server::GetAcceptSocket() const
{
	return AcceptSocket;
}

bool Server::IsRunning() const
{
	return bRunning;
}

NET_THREAD(TickThread)
{
	const auto server = (Server*)parameter;
	if (!server) return NULL;

	LOG_DEBUG(CSTRING("[NET] - Tick thread has been started"));
	while (server->IsRunning())
	{
		server->Tick();
		Kernel32::Sleep(FREQUENZ(server));
	}
	LOG_DEBUG(CSTRING("[NET] - Tick thread has been end"));
	return NULL;
}

NET_THREAD(AcceptorThread)
{
	const auto server = (Server*)parameter;
	if (!server) return NULL;

	LOG_DEBUG(CSTRING("[NET] - Acceptor thread has been started"));
	while (server->IsRunning())
	{
		server->Acceptor();
		Kernel32::Sleep(FREQUENZ(server));
	}
	LOG_DEBUG(CSTRING("[NET] - Acceptor thread has been end"));
	return NULL;
}

bool Server::Run()
{
	if (IsRunning())
		return false;

	// create WSADATA object
	WSADATA wsaData;

	// our sockets for the server
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);

	// address info for the server to listen to
	addrinfo* result = nullptr;
	auto hints = addrinfo();

	// Initialize Winsock
	auto res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0)
	{
		LOG_ERROR(CSTRING("[%s] - WSAStartup failed with error: %d"), SERVERNAME(this), res);
		return false;
	}

	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	const auto Port = std::to_string(Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);
	res = getaddrinfo(NULLPTR, Port.data(), &hints, &result);

	if (res != 0) {
		LOG_ERROR(CSTRING("[%s] - getaddrinfo failed with error: %d"), SERVERNAME(this), res);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	SetListenSocket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));

	if (GetListenSocket() == INVALID_SOCKET) {
		LOG_ERROR(CSTRING("[%s] - socket failed with error: %ld"), SERVERNAME(this), WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;
	res = ioctlsocket(GetListenSocket(), FIONBIO, &iMode);

	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[%s] - ioctlsocket failed with error: %d"), SERVERNAME(this), WSAGetLastError());
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	res = bind(GetListenSocket(), result->ai_addr, static_cast<int>(result->ai_addrlen));

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - bind failed with error: %d"), SERVERNAME(this), WSAGetLastError());
		freeaddrinfo(result);
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// no longer need address information
	freeaddrinfo(result);

	// start listening for new clients attempting to connect
	res = listen(GetListenSocket(), SOMAXCONN);

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - listen failed with error: %d"), SERVERNAME(this), WSAGetLastError());
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// Create all needed Threads
	Thread::Create(TickThread, this);
	Thread::Create(AcceptorThread, this);

	SetRunning(true);
	LOG_SUCCESS(CSTRING("[%s] - started on Port: %d"), SERVERNAME(this), SERVERPORT(this));
	return true;
}

bool Server::Close()
{
	if (!IsRunning())
	{
		LOG_ERROR(CSTRING("[%s] - Can't close server, because server is not running!"), SERVERNAME(this));
		return false;
	}

	SetRunning(false);

	if (GetListenSocket())
		closesocket(GetListenSocket());

	if (GetAcceptSocket())
		closesocket(GetAcceptSocket());

	WSACleanup();

	LOG_DEBUG(CSTRING("[%s] - Closed!"), SERVERNAME(this));
	return true;
}

void Server::SingleSend(NET_PEER peer, const char* data, size_t size, bool& bPreviousSentFailed)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (bPreviousSentFailed)
		return;

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		char* ptr = (char*)data;
		for (size_t it = 0; it < size; ++it)
			ptr[it] = ptr[it] ^ peer->curToken;
	}

	do
	{
		const auto res = send(peer->pSocket, data, static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);
}

void Server::SingleSend(NET_PEER peer, BYTE*& data, size_t size, bool& bPreviousSentFailed)
{
	PEER_NOT_VALID(peer,
		FREE(data);
	return;
	);

	if (bPreviousSentFailed)
	{
		FREE(data);
		return;
	}

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < size; ++it)
			data[it] = data[it] ^ peer->curToken;
	}

	do
	{
		const auto res = send(peer->pSocket, reinterpret_cast<const char*>(data), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);

	FREE(data);
}

void Server::SingleSend(NET_PEER peer, CPOINTER<BYTE>& data, size_t size, bool& bPreviousSentFailed)
{
	PEER_NOT_VALID(peer,
		data.free();
	return;
	);

	if (bPreviousSentFailed)
	{
		data.free();
		return;
	}

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < size; ++it)
			data.get()[it] = data.get()[it] ^ peer->curToken;
	}

	do
	{
		const auto res = send(peer->pSocket, reinterpret_cast<const char*>(data.get()), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
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
void Server::DoSend(NET_PEER peer, const int id, NET_PACKAGE pkg)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
		{
			peer->lastToken = peer->curToken;
			peer->curToken = Net::Coding::FA2::generateToken(peer->fa2_secret, peer->fa2_secret_len, peer->curTime, Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
		}
		else
		{
			peer->lastToken = peer->curToken;
			peer->curToken = Net::Coding::FA2::generateToken(peer->fa2_secret, peer->fa2_secret_len, time(nullptr), Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
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
	if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && peer->cryption.getHandshakeStatus())
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

			LOG_ERROR(CSTRING("Failed to Init AES [0]"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitAES);
			return;
		}

		if (!peer->cryption.RSA)
		{
			Key.free();
			IV.free();

			LOG_ERROR(CSTRING("RSA Object has no instance"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitAES);
			return;
		}

		/* Encrypt AES Keypair using RSA */
		if (!peer->cryption.RSA->encryptBase64(Key.reference().get(), aesKeySize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed Key to encrypt and encode to base64"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_CryptKeyBase64);
			return;
		}

		size_t IVSize = CryptoPP::AES::BLOCKSIZE;
		if (!peer->cryption.RSA->encryptBase64(IV.reference().get(), IVSize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed IV to encrypt and encode to base64"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_CryptIVBase64);
			return;
		}

		/* Crypt Buffer using AES and Encode to Base64 */
		auto dataBufferSize = buffer.GetSize();
		CPOINTER<BYTE> dataBuffer(ALLOC<BYTE>(dataBufferSize + 1));
		memcpy(dataBuffer.get(), buffer.GetString(), dataBufferSize);
		buffer.Flush();
		dataBuffer.get()[dataBufferSize] = '\0';
		aes.encrypt(dataBuffer.get(), dataBufferSize);

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
		SingleSend(peer, NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER), bPreviousSentFailed);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, strlen(NET_PACKAGE_SIZE), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		/* Append Package Key */
		SingleSend(peer, NET_AES_KEY, strlen(NET_AES_KEY), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(peer, KeySizeStr.data(), KeySizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(peer, Key, aesKeySize, bPreviousSentFailed);

		/* Append Package IV */
		SingleSend(peer, NET_AES_IV, strlen(NET_AES_IV), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(peer, IVSizeStr.data(), IVSizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(peer, IV, IVSize, bPreviousSentFailed);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(peer, data.key(), data.keylength() + 1, bPreviousSentFailed);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(peer, data.value(), data.size(), bPreviousSentFailed);
				PKG.DoNotDestruct();
			}
		}

		SingleSend(peer, NET_DATA, strlen(NET_DATA), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
		SingleSend(peer, dataBuffer, dataBufferSize, bPreviousSentFailed);

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER), bPreviousSentFailed);
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
		SingleSend(peer, NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER), bPreviousSentFailed);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, strlen(NET_PACKAGE_SIZE), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(peer, data.key(), data.keylength() + 1, bPreviousSentFailed);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(peer, data.value(), data.size(), bPreviousSentFailed);
				PKG.DoNotDestruct();
			}
		}

		SingleSend(peer, NET_DATA, strlen(NET_DATA), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, strlen(NET_PACKAGE_BRACKET_OPEN), bPreviousSentFailed);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed);

		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			SingleSend(peer, dataBuffer, dataBufferSize, bPreviousSentFailed);
		else
			SingleSend(peer, buffer.GetString(), buffer.GetSize(), bPreviousSentFailed);

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER), bPreviousSentFailed);
	}
}

short Server::Handshake(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return ServerHandshake::peer_not_valid;
	);

	const auto data_size = recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENETDOWN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEFAULT:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENOTCONN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINTR:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINPROGRESS:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENETRESET:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENOTSOCK:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEOPNOTSUPP:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAESHUTDOWN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEWOULDBLOCK:
			peer->network.reset();
			return ServerHandshake::would_block;

		case WSAEMSGSIZE:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINVAL:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAECONNABORTED:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAETIMEDOUT:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAECONNRESET:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		default:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;
		}
	}
	if (data_size == 0)
	{
		peer->network.reset();
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), SERVERNAME(this), peer->IPAddr().get());
		ErasePeer(peer);
		return ServerHandshake::error;
	}
	peer->network.getDataReceive()[data_size] = '\0';

	if (!peer->network.dataValid())
	{
		peer->network.allocData(data_size);
		memcpy(peer->network.getData(), peer->network.getDataReceive(), data_size);
	}
	else
	{
		//* store incomming */
		const auto newBuffer = ALLOC<BYTE>(peer->network.getDataSize() + data_size + 1);
		memcpy(newBuffer, peer->network.getData(), peer->network.getDataSize());
		memcpy(&newBuffer[peer->network.getDataSize()], peer->network.getDataReceive(), data_size);
		peer->network.setDataSize(peer->network.getDataSize() + data_size);
		peer->network.setData(newBuffer); // pointer swap
	}

	peer->network.reset();

	std::string tmp(reinterpret_cast<const char*>(peer->network.getData()));
	const auto pos = tmp.find(CSTRING("GET / HTTP/1.1"));
	if (pos != std::string::npos)
	{
		std::map<std::string, std::string> entries;

		std::istringstream s(reinterpret_cast<const char*>(peer->network.getData()));
		std::string header;
		std::string::size_type end;

		// get headers
		while (std::getline(s, header) && header != CSTRING("\r"))
		{
			if (header[header.size() - 1] != '\r') {
				continue; // ignore malformed header lines?
			}

			header.erase(header.end() - 1);
			end = header.find(CSTRING(": "), 0);

			if (end != std::string::npos)
			{
				const auto key = header.substr(0, end);
				const auto val = header.substr(end + 2);

				entries[key] = val;
			}
		}

		// clear data
		peer->network.clear();

		const auto stringUpdate = CSTRING("Upgrade");

		/* check if we have connected through a browser client */
		if (strcmp(entries[stringUpdate].data(), CSTRING("websocket")) != 0)
			return ServerHandshake::is_websocket;

		return ServerHandshake::is_not_websocket;
	}

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen on Handshake"), SERVERNAME(this), peer->IPAddr().get());

	// clear data
	peer->network.clear();
	return ServerHandshake::error;
}


struct Receive_t
{
	Server* server;
	Server::NET_PEER peer;
};

NET_THREAD(Receive)
{
	const auto param = (Receive_t*)parameter;
	if (!param) return NULL;

	auto peer = param->peer;
	const auto server = param->server;

	do
	{
		peer->setAsync(true);
		const auto res = server->Handshake(peer);
		if (res == ServerHandshake::peer_not_valid)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to invalid socket!"), SERVERNAME(server), peer->IPAddr().get());

			// erase him
			peer->setAsync(false);
			server->ErasePeer(peer);
			return NULL;
		}
		if (res == ServerHandshake::would_block)
		{
			peer->setAsync(false);
			break;
		}
		if (res == ServerHandshake::is_websocket)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to websocket!"), SERVERNAME(server), peer->IPAddr().get());

			// erase him
			peer->setAsync(false);
			server->ErasePeer(peer);
			return NULL;
		}
		if (res == ServerHandshake::error)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake error!"), SERVERNAME(server), peer->IPAddr().get());

			peer->setAsync(false);
			server->ErasePeer(peer);
			return NULL;
		}
		if (res == ServerHandshake::is_not_websocket)
		{
			peer->setAsync(false);
			break;
		}
		peer->setAsync(false);
	} while (true);

	if (server->Isset(NET_OPT_USE_CIPHER) ? server->GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER)
	{
		/* Create new RSA Key Pair */
		peer->cryption.createKeyPair(server->Isset(NET_OPT_CIPHER_RSA_SIZE) ? server->GetOption<size_t>(NET_OPT_CIPHER_RSA_SIZE) : NET_OPT_DEFAULT_RSA_SIZE);

		const auto PublicKey = peer->cryption.RSA->PublicKey();

		Package PKG;
		PKG.Append<const char*>(CSTRING("PublicKey"), PublicKey.get());
		server->NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake, pkg);
	}
	else
	{
		// keep it empty, we get it filled back
		Package PKG;
		server->NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage, pkg);
	}

	while (peer)
	{
		if (!server->IsRunning())
			break;

		server->OnPeerUpdate(peer);

		DWORD restTime = NULL;
		SOCKET_VALID(peer->pSocket)
		{
			peer->setAsync(true);
			restTime = server->DoReceive(peer);
			peer->setAsync(false);
		}
		else
		{
			peer->setAsync(false);
			break;
		}

		Kernel32::Sleep(restTime);
	}

	// wait until thread has finished
	while (peer && peer->bLatency) Kernel32::Sleep(FREQUENZ(server));

	// erase him
	peer->setAsync(false);
	server->ErasePeer(peer);

	delete peer;
	peer = nullptr;
}

void Server::Acceptor()
{
	/* This function manages all the incomming connection */

	// if client waiting, accept the connection and save the socket
	auto client_addr = sockaddr_in();
	socklen_t slen = sizeof(client_addr);
	SetAcceptSocket(accept(GetListenSocket(), (sockaddr*)&client_addr, &slen));

	if (GetAcceptSocket() != INVALID_SOCKET)
	{
		// Set socket options
		for (const auto& entry : socketoption)
		{
			const auto res = _SetSocketOption(GetAcceptSocket(), entry);
			if (res < 0)
				LOG_ERROR(CSTRING("[%s] - Failure on settings socket option { 0x%ld : %i }"), SERVERNAME(this), entry.opt, GetLastError());
		}

		auto peer = CreatePeer(client_addr, GetAcceptSocket());
		const auto param = new Receive_t();
		param->server = this;
		param->peer = peer;
		Thread::Create(Receive, param);
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
DWORD Server::DoReceive(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return FREQUENZ(this);
	);

	SOCKET_NOT_VALID(peer->pSocket)
		return FREQUENZ(this);

	const auto data_size = recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAENETDOWN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEFAULT:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAENOTCONN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEINTR:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEINPROGRESS:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAENETRESET:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAENOTSOCK:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEOPNOTSUPP:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAESHUTDOWN:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEWOULDBLOCK:
			ProcessPackages(peer);
			peer->network.reset();
			return FREQUENZ(this);

		case WSAEMSGSIZE:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAEINVAL:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAECONNABORTED:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAETIMEDOUT:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		case WSAECONNRESET:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);

		default:
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), SERVERNAME(this), peer->IPAddr().get());
			ErasePeer(peer);
			return FREQUENZ(this);
		}
	}
	if (data_size == 0)
	{
		peer->network.reset();
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), SERVERNAME(this), peer->IPAddr().get());
		ErasePeer(peer);
		return FREQUENZ(this);
	}

	if (!peer->network.dataValid())
	{
		peer->network.allocData(data_size);
		memcpy(peer->network.getData(), peer->network.getDataReceive(), data_size);
	}
	else
	{
		if (peer->network.getDataFullSize() > 0
			&& peer->network.getDataSize() + data_size < peer->network.getDataFullSize())
		{
			memcpy(&peer->network.getData()[peer->network.getDataSize()], peer->network.getDataReceive(), data_size);
			peer->network.setDataSize(peer->network.getDataSize() + data_size);
		}
		else
		{
			/* store incomming */
			const auto newBuffer = ALLOC<BYTE>(peer->network.getDataSize() + data_size + 1);
			memcpy(newBuffer, peer->network.getData(), peer->network.getDataSize());
			memcpy(&newBuffer[peer->network.getDataSize()], peer->network.getDataReceive(), data_size);
			newBuffer[peer->network.getDataSize() + data_size] = '\0';
			peer->network.setDataSize(peer->network.getDataSize() + data_size);
			peer->network.setData(newBuffer); // pointer swap
		}
	}

	peer->network.reset();
	ProcessPackages(peer);
	return NULL;
}

void Server::ProcessPackages(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (!peer->network.getDataSize()
		|| peer->network.getDataSize() == INVALID_SIZE)
		return;

	const auto offset = static_cast<int>(strlen(NET_PACKAGE_HEADER)) + static_cast<int>(strlen(NET_PACKAGE_SIZE)); // skip header tags
	if (peer->network.getDataSize() < offset) return;

	auto use_old_token = true;
	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		// shift the first bytes to check if we are using the correct token - using old token
		for (size_t it = 0; it < offset; ++it)
			peer->network.getData()[it] = peer->network.getData()[it] ^ peer->lastToken;

		if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
		{
			// shift back
			for (size_t it = 0; it < offset; ++it)
				peer->network.getData()[it] = peer->network.getData()[it] ^ peer->lastToken;

			if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
			{
				peer->lastToken = peer->curToken;
				peer->curToken = Net::Coding::FA2::generateToken(peer->fa2_secret, peer->fa2_secret_len, peer->curTime, Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
			}
			else
			{
				peer->lastToken = peer->curToken;
				peer->curToken = Net::Coding::FA2::generateToken(peer->fa2_secret, peer->fa2_secret_len, time(nullptr), Isset(NET_OPT_2FA_INTERVAL) ? GetOption<int>(NET_OPT_2FA_INTERVAL) : NET_OPT_DEFAULT_2FA_INTERVAL);
			}

			// shift the first bytes to check if we are using the correct token - using new token
			for (size_t it = 0; it < offset; ++it)
				peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

			// [PROTOCOL] - check header is actually valid
			if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
			{
				LOG_ERROR(CSTRING("[NET] - Frame has no valid header... dropping frame"));
				peer->network.clear();
				DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InvalidFrameHeader);
				return;
			}

			use_old_token = false;
		}
	}
	else
	{
		// [PROTOCOL] - check header is actually valid
		if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
		{
			LOG_ERROR(CSTRING("[NET] - Frame has no valid header... dropping frame"));
			peer->network.clear();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InvalidFrameHeader);
			return;
		}
	}

	// [PROTOCOL] - read data full size from header
	for (size_t i = offset; i < peer->network.getDataSize(); ++i)
	{
		// shift the byte
		if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
			peer->network.getData()[i] = peer->network.getData()[i] ^ (use_old_token ? peer->lastToken : peer->curToken);

		// iterate until we have found the end tag
		if (!memcmp(&peer->network.getData()[i], NET_PACKAGE_BRACKET_CLOSE, 1))
		{
			const auto bDoPreAlloc = peer->network.getDataFullSize() == 0 ? true : false;
			peer->network.SetDataOffset(i);
			const auto size = i - offset - 1;
			CPOINTER<BYTE> sizestr(ALLOC<BYTE>(size + 1));
			memcpy(sizestr.get(), &peer->network.getData()[offset + 1], size);
			sizestr.get()[size] = '\0';
			peer->network.setDataFullSize(strtoull(reinterpret_cast<const char*>(sizestr.get()), nullptr, 10));
			sizestr.free();

			// awaiting more bytes
			if (bDoPreAlloc &&
				peer->network.getDataFullSize() > peer->network.getDataSize())
			{
				// pre-allocate enough space
				const auto newBuffer = ALLOC<BYTE>(peer->network.getDataFullSize() + 1);
				memcpy(newBuffer, peer->network.getData(), peer->network.getDataSize());
				newBuffer[peer->network.getDataFullSize()] = '\0';
				peer->network.setData(newBuffer); // pointer swap

				// shift all the way back
				if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
				{
					for (size_t it = 0; it < i + 1; ++it)
						peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
				}

				return;
			}

			// shift all the way back
			if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
			{
				for (size_t it = 0; it < i + 1; ++it)
					peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
			}

			break;
		}
	}

	// keep going until we have received the entire package
	if (peer->network.getDataSize() < peer->network.getDataFullSize()) return;

	// shift only as much as required
	if (Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA)
	{
		for (size_t it = 0; it < peer->network.getDataFullSize(); ++it)
			peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
	}

	// [PROTOCOL] - check footer is actually valid
	if (memcmp(&peer->network.getData()[peer->network.getDataFullSize() - strlen(NET_PACKAGE_FOOTER)], NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER)) != 0)
	{
		LOG_ERROR(CSTRING("[NET] - Frame has no valid footer... dropping frame"));
		peer->network.clear();
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InvalidFrameFooter);
		return;
	}

	// Execute the package
	ExecutePackage(peer);

	// re-alloc buffer
	const auto leftSize = static_cast<int>(peer->network.getDataSize() - peer->network.getDataFullSize()) > 0 ? peer->network.getDataSize() - peer->network.getDataFullSize() : INVALID_SIZE;
	if (leftSize != INVALID_SIZE
		&& leftSize > 0)
	{
		const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
		memcpy(leftBuffer, &peer->network.getData()[peer->network.getDataFullSize()], leftSize);
		leftBuffer[leftSize] = '\0';
		peer->network.clear();
		peer->network.setData(leftBuffer); // swap pointer
		peer->network.setDataSize(leftSize);
		return;
	}

	peer->network.clear();
}

void Server::ExecutePackage(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	CPOINTER<BYTE> data;
	std::vector<Package_RawData_t> rawData;

	/* Crypt */
	if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && peer->cryption.getHandshakeStatus())
	{
		auto offset = peer->network.getDataOffset() + 1;

		CPOINTER<BYTE> AESKey;
		size_t AESKeySize;

		// look for key tag
		if (!memcmp(&peer->network.getData()[offset], NET_AES_KEY, strlen(NET_AES_KEY)))
		{
			offset += strlen(NET_AES_KEY);

			// read size
			for (auto y = offset; y < peer->network.getDataSize(); ++y)
			{
				if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESKeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESKey = ALLOC<BYTE>(AESKeySize + 1);
			memcpy(AESKey.get(), &peer->network.getData()[offset], AESKeySize);
			AESKey.get()[AESKeySize] = '\0';

			offset += AESKeySize;
		}

		CPOINTER<BYTE> AESIV;
		size_t AESIVSize;

		// look for iv tag
		if (!memcmp(&peer->network.getData()[offset], NET_AES_IV, strlen(NET_AES_IV)))
		{
			offset += strlen(NET_AES_IV);

			// read size
			for (auto y = offset; y < peer->network.getDataSize(); ++y)
			{
				if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESIVSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESIV = ALLOC<BYTE>(AESIVSize + 1);
			memcpy(AESIV.get(), &peer->network.getData()[offset], AESIVSize);
			AESIV.get()[AESIVSize] = '\0';

			offset += AESIVSize;
		}

		if (!peer->cryption.RSA)
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Failure on initializing RSA"));
			return;
		}

		if (!peer->cryption.RSA->decryptBase64(AESKey.reference().get(), AESKeySize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-Key & RSA and Base64"));
			return;
		}

		if (!peer->cryption.RSA->decryptBase64(AESIV.reference().get(), AESIVSize))
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
			if (!memcmp(&peer->network.getData()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
			{
				offset += strlen(NET_RAW_DATA_KEY);

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < peer->network.getDataSize(); ++y)
					{
						if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &peer->network.getData()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&peer->network.getData()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
				{
					offset += strlen(NET_RAW_DATA);

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < peer->network.getDataSize(); ++y)
						{
							if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &peer->network.getData()[offset], packageSize };

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
			if (!memcmp(&peer->network.getData()[offset], NET_DATA, strlen(NET_DATA)))
			{
				offset += strlen(NET_DATA);

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < peer->network.getDataSize(); ++y)
					{
						if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
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
				memcpy(data.get(), &peer->network.getData()[offset], packageSize);
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
			if (offset + strlen(NET_PACKAGE_FOOTER) == peer->network.getDataFullSize())
				break;

		} while (true);
	}
	else
	{
		auto offset = peer->network.getDataOffset() + 1;

		do
		{
			// look for raw data tag
			if (!memcmp(&peer->network.getData()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
			{
				offset += strlen(NET_RAW_DATA_KEY);

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < peer->network.getDataSize(); ++y)
					{
						if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &peer->network.getData()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&peer->network.getData()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
				{
					offset += strlen(NET_RAW_DATA);

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < peer->network.getDataSize(); ++y)
						{
							if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &peer->network.getData()[offset], packageSize };

					if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						DecompressData(entry.value(), entry.size());

					rawData.emplace_back(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&peer->network.getData()[offset], NET_DATA, strlen(NET_DATA)))
			{
				offset += strlen(NET_DATA);

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < peer->network.getDataSize(); ++y)
					{
						if (!memcmp(&peer->network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer->network.getData()[offset + 1], psize);
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
				memcpy(data.get(), &peer->network.getData()[offset], packageSize);
				data.get()[packageSize] = '\0';

				offset += packageSize;

				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					DecompressData(data.reference().get(), packageSize);
			}

			// we have reached the end of reading
			if (offset + strlen(NET_PACKAGE_FOOTER) == peer->network.getDataFullSize())
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

	if (!CheckDataN(peer, id, Content))
		if (!CheckData(peer, id, Content))
		{
			LOG_PEER(CSTRING("[NET] - Frame is not defined"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);
		}

	data.free();
}

void Server::CompressData(BYTE*& data, size_t& size)
{
	/* Compression */
	if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
	{
#ifdef DEBUG
		const auto PrevSize = size;
#endif
		NET_ZLIB::Compress(data, size);
#ifdef DEBUG
		LOG_DEBUG(CSTRING("Compressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

void Server::DecompressData(BYTE*& data, size_t& size)
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

		LOG("%s", copy);

		data = copy; // swap pointer;
#ifdef DEBUG
		LOG_DEBUG(CSTRING("Decompressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

NET_SERVER_BEGIN_DATA_PACKAGE_NATIVE(Server)
NET_SERVER_DEFINE_PACKAGE(RSAHandshake, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake)
NET_SERVER_DEFINE_PACKAGE(VersionPackage, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage)
NET_SERVER_END_DATA_PACKAGE

NET_BEGIN_FNC_PKG(Server, RSAHandshake)
if (!(Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER))
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame, cipher option is been disabled on the server, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	return;
}
if (peer->estabilished)
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame but is already been estabilished, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	return;
}
if (peer->cryption.getHandshakeStatus())
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame which already have been performed, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	return;
}

const auto publicKey = PKG.String(CSTRING("PublicKey"));
if (!publicKey.valid()) // empty
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent an empty handshake frame, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	return;
}

peer->cryption.RSA->SetPublicKey((char*)publicKey.value());

// from now we use the Cryption, synced with Server
peer->cryption.setHandshakeStatus(true);

// RSA Handshake has been finished, keep going with normal process
LOG_PEER(CSTRING("[%s][%s] - Peer ('%s'): has successfully performed a handshake"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());

// keep it empty, we get it filled back
Package Version;
NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage, Version);
NET_END_FNC_PKG

NET_BEGIN_FNC_PKG(Server, VersionPackage)
if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && !peer->cryption.getHandshakeStatus())
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame but have not performed a handshake yet, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Version);
	return;
}
if (peer->estabilished)
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame but is already been estabilished, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Version);
	return;
}

const auto majorVersion = PKG.Int(CSTRING("MajorVersion"));
const auto minorVersion = PKG.Int(CSTRING("MinorVersion"));
const auto revision = PKG.Int(CSTRING("Revision"));
const auto key = PKG.String(CSTRING("Key"));

if (!majorVersion.valid()
	|| !minorVersion.valid()
	|| !revision.valid()
	|| !key.valid())
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame with invalid values"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Versionmismatch);
	return;
}

if ((majorVersion.value() == Version::Major())
	&& (minorVersion.value() == Version::Minor())
	&& (revision.value() == Version::Revision())
	&& strcmp(key.value(), Version::Key().data().data()) == 0)
{
	peer->NetVersionMatched = true;

	Package estabilish;
	NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_EstabilishPackage, estabilish);

	peer->estabilished = true;

	LOG_PEER(CSTRING("[%s][%s] - Peer ('%s'): has successfully been estabilished"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());

	// callback
	OnPeerEstabilished(peer);
}
else
{
	LOG_PEER(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame with different values:\n(%i.%i.%i-%s)"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get(), majorVersion.value(), minorVersion.value(), revision.value(), key.value());

	// version or key missmatch, disconnect peer
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Versionmismatch);
}
NET_END_FNC_PKG

size_t Server::getCountPeers() const
{
	return _CounterPeersTable;
}

bool Server::Create2FASecret(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return false;
	);

	if (!(Isset(NET_OPT_USE_2FA) ? GetOption<bool>(NET_OPT_USE_2FA) : NET_OPT_DEFAULT_USE_2FA))
		return false;

	peer->curTime = time(nullptr);
	if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
	{
		const auto time = Net::Protocol::NTP::Exec(Isset(NET_OPT_NTP_HOST) ? GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
			Isset(NET_OPT_NTP_PORT) ? GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

		if (!time.valid())
		{
			LOG_ERROR(CSTRING("[%s] - critical failure on calling NTP host"), SERVERNAME(this));
			return false;
		}

		peer->curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
	}

	tm tm;
	gmtime_s(&tm, &peer->curTime);
	tm.tm_hour = Net::Util::roundUp(tm.tm_hour, 10);
	tm.tm_min = Net::Util::roundUp(tm.tm_min, 10);
	tm.tm_sec = 0;
	const auto txTm = mktime(&tm);

	const auto strTime = ctime(&txTm);
	peer->fa2_secret_len = strlen(strTime);

	FREE(peer->fa2_secret);
	peer->fa2_secret = ALLOC<byte>(peer->fa2_secret_len + 1);
	memcpy(peer->fa2_secret, strTime, peer->fa2_secret_len);
	peer->fa2_secret[peer->fa2_secret_len] = '\0';
	Net::Coding::Base32::base32_encode(peer->fa2_secret, peer->fa2_secret_len);

	return true;
}
NET_NAMESPACE_END
NET_NAMESPACE_END