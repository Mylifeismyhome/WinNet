#include <NetServer/Server.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/Import/Ws2_32.hpp>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Server)
IPRef::IPRef(const char* pointer)
{
	this->pointer = (char*)pointer;
}

IPRef::~IPRef()
{
	FREE(pointer);
}

const char* IPRef::get() const
{
	return pointer;
}

Server::Server()
{
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);
	SetRunning(false);
	curTime = NULL;
	hSyncClockNTP = nullptr;
	hReSyncClockNTP = nullptr;
	optionBitFlag = NULL;
	socketOptionBitFlag = NULL;
}

Server::~Server()
{
	for (auto& entry : socketoption)
		delete entry;

	socketoption.clear();

	for (auto& entry : option)
		delete entry;

	option.clear();
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
	RSA.generateKeys(size, 3);
	setHandshakeStatus(false);
}

void Server::cryption_t::deleteKeyPair()
{
	RSA.deleteKeys();
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

	// tmp: disabled till linux support for icmp
	//	peer->latency = Net::Protocol::ICMP::Exec(peer->IPAddr().get());

	Timer::SetTime(peer->hCalcLatency, server->Isset(NET_OPT_INTERVAL_LATENCY) ? server->GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY);
	NET_CONTINUE_TIMER;
}

NET_TIMER(NTPSyncClock)
{
	const auto server = (Server*)param;
	if (!server) NET_STOP_TIMER;

	tm* tm = localtime(&server->curTime);
	tm->tm_sec += 1;
	server->curTime = mktime(tm);
	NET_CONTINUE_TIMER;
}

NET_TIMER(NTPReSyncClock)
{
	const auto server = (Server*)param;
	if (!server) NET_STOP_TIMER;

	auto time = Net::Protocol::NTP::Exec(server->Isset(NET_OPT_NTP_HOST) ? server->GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
		server->Isset(NET_OPT_NTP_PORT) ? server->GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

	if (!time.valid())
	{
		LOG_ERROR(CSTRING("[NET] - critical failure on calling NTP host"));
		NET_CONTINUE_TIMER;
	}

	server->curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

	Timer::SetTime(server->hReSyncClockNTP, server->Isset(NET_OPT_NTP_SYNC_INTERVAL) ? server->GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL);
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
	Ws2_32::setsockopt(peer->pSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof tv);

	// Set socket options
	for (const auto& entry : socketoption)
	{
		const auto res = Ws2_32::setsockopt(peer->pSocket, entry->level, entry->opt, entry->value(), entry->optlen());
		if (res == SOCKET_ERROR) LOG_ERROR(CSTRING("[%s] - Following socket option could not been applied { %i : %i }"), SERVERNAME(this), entry->opt, LAST_ERROR);
	}

	if (Isset(NET_OPT_DISABLE_LATENCY_REQUEST) ? GetOption<bool>(NET_OPT_DISABLE_LATENCY_REQUEST) : NET_OPT_DEFAULT_LATENCY_REQUEST)
	{
		const auto _CalcLatency = new CalcLatency_t();
		_CalcLatency->server = this;
		_CalcLatency->peer = peer;
		//peer->hCalcLatency = Timer::Create(CalcLatency, Isset(NET_OPT_INTERVAL_LATENCY) ? GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY, _CalcLatency, true);
	}

	if (CreateTOTPSecret(peer))
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): successfully created TOTP-Hash"), SERVERNAME(this), peer->IPAddr().get());

	IncreasePeersCounter();

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): connected"), SERVERNAME(this), peer->IPAddr().get());

	// callback
	OnPeerConnect(peer);

	return peer;
}

bool Server::ErasePeer(NET_PEER peer, bool clear)
{
	PEER_NOT_VALID(peer,
		return false;
	);

	if (clear)
	{
		if (peer->hCalcLatency)
		{
			// stop latency interval
			//Timer::WaitSingleObjectStopped(peer->hCalcLatency);
			peer->hCalcLatency = nullptr;
		}

		// callback
#ifdef BUILD_LINUX
		OnPeerDisconnect(peer, errno);
#else
		OnPeerDisconnect(peer, Ws2_32::WSAGetLastError());
#endif

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): disconnected"), SERVERNAME(this), peer->IPAddr().get());

		peer->clear();

		DecreasePeersCounter();

		return true;
	}

	// close endpoint
	SOCKET_VALID(peer->pSocket)
	{
		Ws2_32::closesocket(peer->pSocket);
		peer->pSocket = INVALID_SOCKET;
	}

	peer->bErase = true;
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
	NetVersionMatched = false;
	bErase = false;
	latency = -1;
	hCalcLatency = nullptr;

	network.clear();
	network.reset();

	cryption.deleteKeyPair();

	FREE(totp_secret);
	totp_secret_len = NULL;
	curToken = NULL;
	lastToken = NULL;
}

typeLatency Server::NET_IPEER::getLatency() const
{
	return latency;
}

IPRef Server::NET_IPEER::IPAddr() const
{
	const auto buf = ALLOC<char>(INET_ADDRSTRLEN);
	return IPRef(Ws2_32::inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN));
}

void Server::DisconnectPeer(NET_PEER peer, const int code, const bool skipNotify)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (!skipNotify)
	{
		Net::Package PKG;
		PKG.Append(CSTRING("code"), code);
		NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_ClosePackage, pkg);
	}

	if (code == 0)
	{
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): has been disconnected"), SERVERNAME(this), peer->IPAddr().get());
	}
	else
	{
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): has been disconnected, reason: %s"), SERVERNAME(this), peer->IPAddr().get(), Net::Codes::NetGetErrorMessage(code));
	}

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

	LOG_DEBUG(CSTRING("[%s] - Tick thread has been started"), SERVERNAME(server));
	while (server->IsRunning())
	{
		server->Tick();
#ifdef BUILD_LINUX
		usleep(FREQUENZ(server));
#else
		Kernel32::Sleep(FREQUENZ(server));
#endif
	}
	LOG_DEBUG(CSTRING("[%s] - Tick thread has been end"), SERVERNAME(server));
	return NULL;
}

NET_THREAD(AcceptorThread)
{
	const auto server = (Server*)parameter;
	if (!server) return NULL;

	LOG_DEBUG(CSTRING("[%s] - Acceptor thread has been started"), SERVERNAME(server));
	while (server->IsRunning())
	{
		server->Acceptor();
#ifdef BUILD_LINUX
		usleep(FREQUENZ(server));
#else
		Kernel32::Sleep(FREQUENZ(server));
#endif
	}
	LOG_DEBUG(CSTRING("[%s] - Acceptor thread has been end"), SERVERNAME(server));
	return NULL;
}

bool Server::Run()
{
	if (IsRunning())
		return false;

	// our sockets for the server
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);

	// address info for the server to listen to
	addrinfo* result = nullptr;
	int res = 0;

#ifndef BUILD_LINUX
	WSADATA wsaData;
	res = Ws2_32::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != NULL)
	{
		LOG_ERROR(CSTRING("[%s] - WSAStartup has been failed with error: %d"), SERVERNAME(this), res);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		LOG_ERROR(CSTRING("[%s] - Could not find a usable version of Winsock.dll"), SERVERNAME(this));
		Ws2_32::WSACleanup();
		return false;
	}
#endif

	// set address information
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	const auto Port = std::to_string(Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);
	res = Ws2_32::getaddrinfo(NULLPTR, Port.data(), &hints, &result);

	if (res != 0) {
		LOG_ERROR(CSTRING("[%s] - getaddrinfo failed with error: %d"), SERVERNAME(this), res);
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Create a SOCKET for connecting to server
	SetListenSocket(Ws2_32::socket(result->ai_family, result->ai_socktype, result->ai_protocol));

	if (GetListenSocket() == INVALID_SOCKET) {
		LOG_ERROR(CSTRING("[%s] - socket failed with error: %ld"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::freeaddrinfo(result);
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Set the mode of the socket to be nonblocking
	unsigned long iMode = 1;
	res = Ws2_32::ioctlsocket(GetListenSocket(), FIONBIO, &iMode);

	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[%s] - ioctlsocket failed with error: %d"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::closesocket(GetListenSocket());
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Setup the TCP listening socket
	res = Ws2_32::bind(GetListenSocket(), result->ai_addr, static_cast<int>(result->ai_addrlen));

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - bind failed with error: %d"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::freeaddrinfo(result);
		Ws2_32::closesocket(GetListenSocket());
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// no longer need address information
	Ws2_32::freeaddrinfo(result);

	// start listening for new clients attempting to connect
	res = Ws2_32::listen(GetListenSocket(), SOMAXCONN);

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - listen failed with error: %d"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::closesocket(GetListenSocket());
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Create all needed Threads
	// spawn timer thread to sync clock with ntp - only effects having 2-step enabled
	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		curTime = time(nullptr);
		if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
		{
			auto time = Net::Protocol::NTP::Exec(Isset(NET_OPT_NTP_HOST) ? GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
				Isset(NET_OPT_NTP_PORT) ? GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

			if (!time.valid())
			{
				LOG_ERROR(CSTRING("[%s] - critical failure on calling NTP host"), SERVERNAME(this));
				return false;
			}

			curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

			hSyncClockNTP = Timer::Create(NTPSyncClock, 1000, this);
			hReSyncClockNTP = Timer::Create(NTPReSyncClock, Isset(NET_OPT_NTP_SYNC_INTERVAL) ? GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL, this);
		}
	}

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

	if (hSyncClockNTP)
	{
		Timer::WaitSingleObjectStopped(hSyncClockNTP);
		hSyncClockNTP = nullptr;
	}

	if (hReSyncClockNTP)
	{
		Timer::WaitSingleObjectStopped(hReSyncClockNTP);
		hReSyncClockNTP = nullptr;
	}

	curTime = NULL;

	SetRunning(false);

	if (GetListenSocket())
		Ws2_32::closesocket(GetListenSocket());

	if (GetAcceptSocket())
		Ws2_32::closesocket(GetAcceptSocket());

#ifndef BUILD_LINUX
	Ws2_32::WSACleanup();
#endif

	LOG_DEBUG(CSTRING("[%s] - Closed!"), SERVERNAME(this));
	return true;
}

void Server::SingleSend(NET_PEER peer, const char* data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (bPreviousSentFailed)
		return;

	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		char* ptr = (char*)data;
		for (size_t it = 0; it < size; ++it)
			ptr[it] = ptr[it] ^ sendToken;
	}

	do
	{
		const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
		if (res == SOCKET_ERROR)
		{
#ifdef BUILD_LINUX
			if (errno == EWOULDBLOCK)
			{
				usleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				ErasePeer(peer);
				if (ERRNO_ERROR_TRIGGERED) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
				return;
			}
#else
			if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Kernel32::Sleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				ErasePeer(peer);
				if (Ws2_32::WSAGetLastError() != 0) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
				return;
			}
#endif
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);
}

void Server::SingleSend(NET_PEER peer, BYTE*& data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
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

	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		for (size_t it = 0; it < size; ++it)
			data[it] = data[it] ^ sendToken;
	}

	do
	{
		const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
		if (res == SOCKET_ERROR)
		{
#ifdef BUILD_LINUX
			if (errno == EWOULDBLOCK)
			{
				usleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				FREE(data);
				ErasePeer(peer);
				if (ERRNO_ERROR_TRIGGERED) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
				return;
			}
#else
			if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Kernel32::Sleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				FREE(data);
				ErasePeer(peer);
				if (Ws2_32::WSAGetLastError() != 0) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
				return;
			}
#endif
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);

	FREE(data);
}

void Server::SingleSend(NET_PEER peer, CPOINTER<BYTE>& data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
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

	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		for (size_t it = 0; it < size; ++it)
			data.get()[it] = data.get()[it] ^ sendToken;
	}

	do
	{
		const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(data.get()), size, MSG_NOSIGNAL);
		if (res == SOCKET_ERROR)
		{
#ifdef BUILD_LINUX
			if (errno == EWOULDBLOCK)
			{
				usleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				data.free();
				ErasePeer(peer);
				if (ERRNO_ERROR_TRIGGERED) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
				return;
			}
#else
			if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Kernel32::Sleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				data.free();
				ErasePeer(peer);
				if (Ws2_32::WSAGetLastError() != 0) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
				return;
			}
#endif
		}
		if (res < 0)
			break;

		size -= res;
			} while (size > 0);

			data.free();
		}

void Server::SingleSend(NET_PEER peer, Package_RawData_t& data, bool& bPreviousSentFailed, const uint32_t sendToken)
{
	if (!data.valid()) return;

	PEER_NOT_VALID(peer,
		data.free();
	return;
	);

	if (bPreviousSentFailed)
	{
		data.free();
		return;
	}

	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		for (size_t it = 0; it < data.size(); ++it)
			data.value()[it] = data.value()[it] ^ sendToken;
	}

	size_t size = data.size();
	do
	{
		const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(data.value()), size, MSG_NOSIGNAL);
		if (res == SOCKET_ERROR)
		{
#ifdef BUILD_LINUX
			if (errno == EWOULDBLOCK)
			{
				usleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				data.free();
				ErasePeer(peer);
				if (ERRNO_ERROR_TRIGGERED) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
				return;
			}
#else
			if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
			{
				Kernel32::Sleep(FREQUENZ(this));
				continue;
			}
			else
			{
				bPreviousSentFailed = true;
				data.free();
				ErasePeer(peer);
				if (Ws2_32::WSAGetLastError() != 0) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
				return;
			}
#endif
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);

	data.free();
}

/*
*							Visualisation of package structure in NET
*	---------------------------------------------------------------------------------------------------------------------------------
*				CRYPTED VERSION					|		NON-CRYPTED VERSION
*	---------------------------------------------------------------------------------------------------------------------------------
*	{KEYWORD}{SIZE}DATA
*	---------------------------------------------------------------------------------------------------------------------------------
*	{BEGIN PACKAGE}								*		{BEGIN PACKAGE}
*		{PACKAGE SIZE}{...}						*			{PACKAGE SIZE}{...}
*			{KEY}{...}...								*						-
*			{IV}{...}...									*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...						*				{RAW DATA}{...}...
*			{DATA}{...}...								*				{DATA}{...}...
*	{END PACKAGE}									*		{END PACKAGE}
*
*/
void Server::DoSend(NET_PEER peer, const int id, NET_PACKAGE pkg)
{
	PEER_NOT_VALID(peer,
		return;
	);

	std::lock_guard<std::recursive_mutex> guard(peer->network._mutex_send);

	uint32_t sendToken = INVALID_UINT_SIZE;
	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
		sendToken = Net::Coding::TOTP::generateToken(peer->totp_secret, peer->totp_secret_len, Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP ? curTime : time(nullptr), Isset(NET_OPT_TOTP_INTERVAL) ? (int)(GetOption<int>(NET_OPT_TOTP_INTERVAL) / 2) : (int)(NET_OPT_DEFAULT_TOTP_INTERVAL / 2));

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

	auto dataBufferSize = buffer.GetLength();
	CPOINTER<BYTE> dataBuffer(ALLOC<BYTE>(dataBufferSize + 1));
	memcpy(dataBuffer.get(), buffer.GetString(), dataBufferSize);
	dataBuffer.get()[dataBufferSize] = '\0';
	buffer.Flush();

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

		if (!aes.init(reinterpret_cast<char*>(Key.get()), reinterpret_cast<char*>(IV.get())))
		{
			Key.free();
			IV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitAES);
			return;
		}

		/* Encrypt AES Keypair using RSA */
		if (!peer->cryption.RSA.encryptBase64(Key.reference().get(), aesKeySize))
		{
			Key.free();
			IV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_CryptKeyBase64);
			return;
		}

		size_t IVSize = CryptoPP::AES::BLOCKSIZE;
		if (!peer->cryption.RSA.encryptBase64(IV.reference().get(), IVSize))
		{
			Key.free();
			IV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_CryptIVBase64);
			return;
		}

		/* Crypt Buffer using AES and Encode to Base64 */
		aes.encrypt(dataBuffer.get(), dataBufferSize);

		if (PKG.HasRawData())
		{
			std::vector<Package_RawData_t>& rawData = PKG.GetRawData();
			for (auto& data : rawData)
				aes.encrypt(data.value(), data.size());
		}

		/* Compression */
		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
		{
			/* Compress Data */
			//CompressData(dataBuffer.reference().get(), dataBufferSize, true);

			/* Compress Raw Data */
			if (PKG.HasRawData())
			{
				for (auto& entry : PKG.GetRawData())
				{
					CompressData(entry.value(), entry.size());
					entry.set_free(true);
				}
			}
		}

		combinedSize = dataBufferSize + NET_PACKAGE_HEADER_LEN + NET_PACKAGE_SIZE_LEN + NET_DATA_LEN + NET_PACKAGE_FOOTER_LEN + NET_AES_KEY_LEN + NET_AES_IV_LEN + aesKeySize + IVSize + 8;

		// Append Raw data package size
		if (PKG.HasRawData()) combinedSize += PKG.GetRawDataFullSize();

		std::string dataSizeStr = std::to_string(dataBufferSize);
		combinedSize += dataSizeStr.length();

		const auto KeySizeStr = std::to_string(aesKeySize);
		combinedSize += KeySizeStr.length();

		const auto IVSizeStr = std::to_string(IVSize);
		combinedSize += IVSizeStr.length();

		const auto EntirePackageSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

		auto bPreviousSentFailed = false;

		/* Append Package Header */
		SingleSend(peer, NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN, bPreviousSentFailed, sendToken);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, NET_PACKAGE_SIZE_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

		/* Append Package Key */
		SingleSend(peer, NET_AES_KEY, NET_AES_KEY_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, KeySizeStr.data(), KeySizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, Key, aesKeySize, bPreviousSentFailed, sendToken);

		/* Append Package IV */
		SingleSend(peer, NET_AES_IV, NET_AES_IV_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, IVSizeStr.data(), IVSizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, IV, IVSize, bPreviousSentFailed, sendToken);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			for (auto& data : PKG.GetRawData())
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

				const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(peer, data.key(), strlen(data.key()) + 1, bPreviousSentFailed, sendToken);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(peer, data, bPreviousSentFailed, sendToken);

				data.set_free(false);
			}
		}

		SingleSend(peer, NET_DATA, NET_DATA_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, dataBuffer, dataBufferSize, bPreviousSentFailed, sendToken);

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, NET_PACKAGE_FOOTER_LEN, bPreviousSentFailed, sendToken);
	}
	else
	{
		/* Compression */
		if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
		{
			/* Compress Data */
			//CompressData(dataBuffer.reference().get(), dataBufferSize);

			/* Compress Raw Data */
			if (PKG.HasRawData())
			{
				for (auto& entry : PKG.GetRawData())
				{
					CompressData(entry.value(), entry.size());
					entry.set_free(true);
				}
			}
		}

		combinedSize = dataBufferSize + NET_PACKAGE_HEADER_LEN + NET_PACKAGE_SIZE_LEN + NET_DATA_LEN + NET_PACKAGE_FOOTER_LEN + 4;

		// Append Raw data package size
		if (PKG.HasRawData()) combinedSize += PKG.GetRawDataFullSize();

		std::string dataSizeStr = std::to_string(dataBufferSize);
		combinedSize += dataSizeStr.length();

		const auto EntirePackageSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

		auto bPreviousSentFailed = false;

		/* Append Package Header */
		SingleSend(peer, NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN, bPreviousSentFailed, sendToken);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, NET_PACKAGE_SIZE_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			for (auto& data : PKG.GetRawData())
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

				const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(peer, data.key(), strlen(data.key()) + 1, bPreviousSentFailed, sendToken);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(peer, data, bPreviousSentFailed, sendToken);

				data.set_free(false);
			}
		}

		SingleSend(peer, NET_DATA, NET_DATA_LEN, bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, strlen(NET_PACKAGE_BRACKET_OPEN), bPreviousSentFailed, sendToken);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed, sendToken);
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
		SingleSend(peer, dataBuffer, dataBufferSize, bPreviousSentFailed, sendToken);

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, NET_PACKAGE_FOOTER_LEN, bPreviousSentFailed, sendToken);
	}
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

	if (server->Isset(NET_OPT_USE_CIPHER) ? server->GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER)
	{
		/* Create new RSA Key Pair */
		peer->cryption.createKeyPair(server->Isset(NET_OPT_CIPHER_RSA_SIZE) ? server->GetOption<size_t>(NET_OPT_CIPHER_RSA_SIZE) : NET_OPT_DEFAULT_RSA_SIZE);

		const auto PublicKey = peer->cryption.RSA.publicKey();

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

	while (peer && peer->pSocket != INVALID_SOCKET)
	{
		if (!server->IsRunning()) break;
		if (peer->bErase) break;

		server->OnPeerUpdate(peer);

		const auto restTime = server->DoReceive(peer);

#ifdef BUILD_LINUX
		usleep(restTime);
#else
		Kernel32::Sleep(restTime);
#endif
	}

	// erase him
	server->ErasePeer(peer, true);

	delete peer;
	peer = nullptr;
	return NULL;
}

void Server::Acceptor()
{
	/* This function manages all the incomming connection */

	// if client waiting, accept the connection and save the socket
	auto client_addr = sockaddr_in();
	socklen_t slen = sizeof(client_addr);
	SetAcceptSocket(Ws2_32::accept(GetListenSocket(), (sockaddr*)&client_addr, &slen));

	if (GetAcceptSocket() != INVALID_SOCKET)
	{
		const auto param = new Receive_t();
		param->server = this;
		param->peer = CreatePeer(client_addr, GetAcceptSocket());
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
*			{KEY}{...}...								*						-
*			{IV}{...}...									*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...						*				{RAW DATA}{...}...
*			{DATA}{...}...								*				{DATA}{...}...
*	{END PACKAGE}									*		{END PACKAGE}
*
 */
DWORD Server::DoReceive(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return FREQUENZ(this);
	);

	SOCKET_NOT_VALID(peer->pSocket)
		return FREQUENZ(this);

	auto data_size = Ws2_32::recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
#ifdef BUILD_LINUX
		if (errno != EWOULDBLOCK)
#else
		if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
		{
			peer->network.reset();
			ErasePeer(peer);

#ifdef BUILD_LINUX
			if (ERRNO_ERROR_TRIGGERED) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
			if (Ws2_32::WSAGetLastError() != 0) LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

			return FREQUENZ(this);
		}

		ProcessPackages(peer);
		peer->network.reset();
		return FREQUENZ(this);
	}

	// graceful disconnect
	if (data_size == 0)
	{
		peer->network.reset();
		ErasePeer(peer);
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), SERVERNAME(this), peer->IPAddr().get());
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

bool Server::ValidHeader(NET_PEER peer, bool& use_old_token)
{
	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		// shift the first bytes to check if we are using the correct token - using old token
		for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
			peer->network.getData()[it] = peer->network.getData()[it] ^ peer->lastToken;

		if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN) != 0)
		{
			// shift back
			for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
				peer->network.getData()[it] = peer->network.getData()[it] ^ peer->lastToken;

			// shift the first bytes to check if we are using the correct token - using cur token
			for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
				peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

			if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN) != 0)
			{
				// shift back
				for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
					peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

				peer->lastToken = peer->curToken;
				peer->curToken = Net::Coding::TOTP::generateToken(peer->totp_secret, peer->totp_secret_len, Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP ? curTime : time(nullptr), Isset(NET_OPT_TOTP_INTERVAL) ? (int)(GetOption<int>(NET_OPT_TOTP_INTERVAL) / 2) : (int)(NET_OPT_DEFAULT_TOTP_INTERVAL / 2));

				// shift the first bytes to check if we are using the correct token - using new token
				for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
					peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

				// [PROTOCOL] - check header is actually valid
				if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN) != 0)
				{
					peer->network.clear();
					DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InvalidFrameHeader);
					return false;
				}

				// sift back using new token
				for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
					peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

				use_old_token = false;
			}
			else
			{
				// sift back using cur token
				for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
					peer->network.getData()[it] = peer->network.getData()[it] ^ peer->curToken;

				use_old_token = false;
			}
		}
		else
		{
			// sift back using old token
			for (size_t it = 0; it < NET_PACKAGE_HEADER_LEN; ++it)
				peer->network.getData()[it] = peer->network.getData()[it] ^ peer->lastToken;
		}
	}
	else
	{
		// [PROTOCOL] - check header is actually valid
		if (memcmp(&peer->network.getData()[0], NET_PACKAGE_HEADER, NET_PACKAGE_HEADER_LEN) != 0)
		{
			peer->network.clear();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InvalidFrameHeader);
			return false;
		}
	}

	return true;
}

void Server::ProcessPackages(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (!peer->network.getDataSize())
		return;

	if (peer->network.getDataSize() == INVALID_SIZE)
		return;

	if (peer->network.getDataSize() < NET_PACKAGE_HEADER_LEN) return;

	auto use_old_token = true;
	bool already_checked = false;

	// [PROTOCOL] - read data full size from header
	if (!peer->network.getDataFullSize() || peer->network.getDataFullSize() == INVALID_SIZE)
	{
		already_checked = true;
		if (!ValidHeader(peer, use_old_token)) return;

		const size_t start = NET_PACKAGE_HEADER_LEN + NET_PACKAGE_SIZE_LEN + 1;
		for (size_t i = start; i < peer->network.getDataSize(); ++i)
		{
			// shift the bytes
			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
				peer->network.getData()[i] = peer->network.getData()[i] ^ (use_old_token ? peer->lastToken : peer->curToken);

			// iterate until we have found the end tag
			if (!memcmp(&peer->network.getData()[i], NET_PACKAGE_BRACKET_CLOSE, 1))
			{
				peer->network.SetDataOffset(i);
				const auto size = i - start;
				char* end = (char*)peer->network.getData()[start] + size;
				peer->network.setDataFullSize(strtoull((const char*)&peer->network.getData()[start], &end, 10));

				// awaiting more bytes
				if (peer->network.getDataFullSize() > peer->network.getDataSize())
				{
					// pre-allocate enough space
					const auto newBuffer = ALLOC<BYTE>(peer->network.getDataFullSize() + 1);
					memcpy(newBuffer, peer->network.getData(), peer->network.getDataSize());
					newBuffer[peer->network.getDataFullSize()] = '\0';
					peer->network.setData(newBuffer); // pointer swap

					// shift all the way back
					if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
					{
						for (size_t it = start; it < i + 1; ++it)
							peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
					}

					return;
				}

				// shift all the way back
				if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
				{
					for (size_t it = start; it < i + 1; ++it)
						peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
				}

				break;
			}
		}
	}

	// keep going until we have received the entire package
	if (!peer->network.getDataFullSize() || peer->network.getDataFullSize() == INVALID_SIZE || peer->network.getDataSize() < peer->network.getDataFullSize()) return;

	if (!already_checked)
		if (!ValidHeader(peer, use_old_token)) return;

	// shift only as much as required
	if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
	{
		for (size_t it = 0; it < peer->network.getDataFullSize(); ++it)
			peer->network.getData()[it] = peer->network.getData()[it] ^ (use_old_token ? peer->lastToken : peer->curToken);
	}

	// [PROTOCOL] - check footer is actually valid
	if (memcmp(&peer->network.getData()[peer->network.getDataFullSize() - NET_PACKAGE_FOOTER_LEN], NET_PACKAGE_FOOTER, NET_PACKAGE_FOOTER_LEN) != 0)
	{
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
	Package Content;

	/* Crypt */
	if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && peer->cryption.getHandshakeStatus())
	{
		auto offset = peer->network.getDataOffset() + 1;

		CPOINTER<BYTE> AESKey;
		size_t AESKeySize;

		// look for key tag
		if (!memcmp(&peer->network.getData()[offset], NET_AES_KEY, NET_AES_KEY_LEN))
		{
			offset += NET_AES_KEY_LEN;

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
		if (!memcmp(&peer->network.getData()[offset], NET_AES_IV, NET_AES_IV_LEN))
		{
			offset += NET_AES_IV_LEN;

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

		if (!peer->cryption.RSA.decryptBase64(AESKey.reference().get(), AESKeySize))
		{
			AESKey.free();
			AESIV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptKeyBase64);
			return;
		}

		if (!peer->cryption.RSA.decryptBase64(AESIV.reference().get(), AESIVSize))
		{
			AESKey.free();
			AESIV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptIVBase64);
			return;
		}

		NET_AES aes;
		if (!aes.init(reinterpret_cast<char*>(AESKey.get()), reinterpret_cast<char*>(AESIV.get())))
		{
			AESKey.free();
			AESIV.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitAES);
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

					Package_RawData_t entry = { (char*)key.get(), &peer->network.getData()[offset], packageSize, false };

					/* Decompression */
					if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					{
						DecompressData(entry.value(), entry.value(), entry.size(), true);
						entry.set_free(true);
					}

					/* decrypt aes */
					if (!aes.decrypt(entry.value(), entry.size()))
					{
						DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptAES);
						return;
					}

					Content.AppendRawData(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&peer->network.getData()[offset], NET_DATA, NET_DATA_LEN))
			{
				offset += NET_DATA_LEN;

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
				size_t dataSize = packageSize;
				data = ALLOC<BYTE>(dataSize + 1);
				memcpy(data.get(), &peer->network.getData()[offset], dataSize);
				data.get()[dataSize] = '\0';

				offset += packageSize;

				/* Decompression */
			//	if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				//	DecompressData(data.reference().get(), packageSize, true);

				/* decrypt aes */
				if (!aes.decrypt(data.get(), dataSize))
				{
					data.free();
					DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptAES);
					return;
				}
			}

			// we have reached the end of reading
			if (offset + NET_PACKAGE_FOOTER_LEN == peer->network.getDataFullSize())
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

					Package_RawData_t entry = { (char*)key.get(), &peer->network.getData()[offset], packageSize, false };

					/* Decompression */
					if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					{
						DecompressData(entry.value(), entry.value(), entry.size(), true);
						entry.set_free(true);
					}

					Content.AppendRawData(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&peer->network.getData()[offset], NET_DATA, NET_DATA_LEN))
			{
				offset += NET_DATA_LEN;

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

				/* Decompression */
				//if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
					//DecompressData(data.reference().get(), packageSize);
			}

			// we have reached the end of reading
			if (offset + NET_PACKAGE_FOOTER_LEN == peer->network.getDataFullSize())
				break;

		} while (true);
	}

	if (!data.valid())
	{
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DataInvalid);
		return;
	}

	Package PKG;
	PKG.Parse(reinterpret_cast<char*>(data.get()));
	if (!PKG.GetPackage().HasMember(CSTRING("ID")))
	{
		data.free();
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_NoMemberID);
		return;
	}

	const auto id = PKG.GetPackage().FindMember(CSTRING("ID"))->value.GetInt();
	if (id < 0)
	{
		data.free();
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_MemberIDInvalid);
		return;
	}

	if (!PKG.GetPackage().HasMember(CSTRING("CONTENT")))
	{
		data.free();
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_NoMemberContent);
		return;
	}

	if (!PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.IsNull())
		Content.SetPackage(PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.GetObject());

	if (!CheckDataN(peer, id, Content))
		if (!CheckData(peer, id, Content))
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);

	data.free();
}

void Server::CompressData(BYTE*& data, size_t& size)
{
#ifdef DEBUG
	const auto PrevSize = size;
#endif
	NET_ZLIB::Compress(data, size);
#ifdef DEBUG
	LOG_DEBUG(CSTRING("[%s] - Compressed data from size %llu to %llu"), SERVERNAME(this), PrevSize, size);
#endif
}

void Server::CompressData(BYTE*& data, BYTE*& out, size_t& size, const bool skip_free)
{
#ifdef DEBUG
	const auto PrevSize = size;
#endif
	NET_ZLIB::Compress(data, out, size, ZLIB_CompressionLevel::BEST_COMPRESSION, skip_free);
#ifdef DEBUG
	LOG_DEBUG(CSTRING("[%s] - Compressed data from size %llu to %llu"), SERVERNAME(this), PrevSize, size);
#endif
}

void Server::DecompressData(BYTE*& data, size_t& size)
{
#ifdef DEBUG
	const auto PrevSize = size;
#endif
	NET_ZLIB::Decompress(data, size);
#ifdef DEBUG
	LOG_DEBUG(CSTRING("[%s] - Decompressed data from size %llu to %llu"), SERVERNAME(this), PrevSize, size);
#endif
}

void Server::DecompressData(BYTE*& data, BYTE*& out, size_t& size, const bool skip_free)
{
#ifdef DEBUG
	const auto PrevSize = size;
#endif
	NET_ZLIB::Decompress(data, out, size, skip_free);
#ifdef DEBUG
	LOG_DEBUG(CSTRING("[%s] - Decompressed data from size %llu to %llu"), SERVERNAME(this), PrevSize, size);
#endif
}

NET_SERVER_BEGIN_DATA_PACKAGE_NATIVE(Server)
NET_SERVER_DEFINE_PACKAGE(RSAHandshake, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake)
NET_SERVER_DEFINE_PACKAGE(VersionPackage, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage)
NET_SERVER_END_DATA_PACKAGE

NET_BEGIN_FNC_PKG(Server, RSAHandshake)
if (!(Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER))
{
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame, cipher option is been disabled on the server, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	return;
}
if (peer->estabilished)
{
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame but is already been estabilished, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	return;
}
if (peer->cryption.getHandshakeStatus())
{
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a handshake frame which already have been performed, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	return;
}

const auto publicKey = PKG.String(CSTRING("PublicKey"));
if (!publicKey.valid()) // empty
{
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Handshake);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent an empty handshake frame, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	return;
}

peer->cryption.RSA.setPublicKey(publicKey.value());

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
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Version);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame but have not performed a handshake yet, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
	return;
}
if (peer->estabilished)
{
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Version);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame but is already been estabilished, rejecting the frame"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
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
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Versionmismatch);
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent a version frame with invalid values"), SERVERNAME(this), FUNCTION_NAME, peer->IPAddr().get());
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

bool Server::CreateTOTPSecret(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return false;
	);

	if (!(Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP))
		return false;

#ifdef BUILD_LINUX
	struct tm* tm = nullptr;
	tm = gmtime(&curTime);
	tm->tm_hour = Net::Util::roundUp(tm->tm_hour, 10);
	tm->tm_min = Net::Util::roundUp(tm->tm_min, 10);
	tm->tm_sec = 0;
	const auto txTm = mktime(tm);
#else
	tm tm;
	gmtime_s(&tm, &curTime);
	tm.tm_hour = Net::Util::roundUp(tm.tm_hour, 10);
	tm.tm_min = Net::Util::roundUp(tm.tm_min, 10);
	tm.tm_sec = 0;
	const auto txTm = mktime(&tm);
#endif

	const auto strTime = ctime(&txTm);
	peer->totp_secret_len = strlen(strTime);

	FREE(peer->totp_secret);
	peer->totp_secret = ALLOC<byte>(peer->totp_secret_len + 1);
	memcpy(peer->totp_secret, strTime, peer->totp_secret_len);
	peer->totp_secret[peer->totp_secret_len] = '\0';
	Net::Coding::Base32::encode(peer->totp_secret, peer->totp_secret_len);

	peer->curToken = NULL;
	peer->lastToken = NULL;

	return true;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
