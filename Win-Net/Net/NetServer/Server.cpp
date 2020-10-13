#include "server.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Server)
static void LatencyThread(Server::NET_PEER peer)
{
	if (!peer)
		return;

	const auto ip = peer.IPAddr();
	ICMP _icmp(ip.get());
	_icmp.execute();

	peer.latency = _icmp.getLatency();
}

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
	NetLoadErrorCodes();
	SetAllToDefault();
}

Server::~Server()
{
}

void Server::SetAllToDefault()
{
	strcpy_s(sServerName, DEFAULT_SERVER_SERVERNAME);
	sServerPort = DEFAULT_SERVER_SERVERPORT;
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);
	sfrequenz = DEFAULT_SERVER_FREQUENZ;
	DoExit = NULL;

	sMaxThreads = DEFAULT_SERVER_MAX_THREADS;
	sRSAKeySize = DEFAULT_SERVER_RSA_KEY_SIZE;
	sAESKeySize = DEFAULT_SERVER_AES_KEY_SIZE;
	sCryptPackage = DEFAULT_SERVER_CRYPT_PACKAGES;
	sCompressPackage = DEFAULT_SERVER_COMPRESS_PACKAGES;
	sTCPReadTimeout = DEFAULT_SERVER_TCP_READ_TIMEOUT;
	sCalcLatencyInterval = DEFAULT_SERVER_CALC_LATENCY_INTERVAL;

	SetRunning(false);

	LOG_DEBUG(CSTRING("---------- SERVER DEFAULT SETTINGS ----------"));
	LOG_DEBUG(CSTRING("Refresh-Frequenz has been set to default value of %lld"), sfrequenz);
	LOG_DEBUG(CSTRING("Max Threads has been set to default value of %i"), sMaxThreads);
	LOG_DEBUG(CSTRING("RSA Key size has been set to default value of %llu"), sRSAKeySize);
	LOG_DEBUG(CSTRING("AES Key size has been set to default value of %llu"), sAESKeySize);
	LOG_DEBUG(CSTRING("Crypt Package has been set to default value of %s"), sCryptPackage ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Compress Package has been set to default value of %s"), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("TCP Read timeout has been set to default value of %i"), sTCPReadTimeout);
	LOG_DEBUG(CSTRING("Calculate latency interval has been set to default value of %i"), sCalcLatencyInterval);
	LOG_DEBUG(CSTRING("---------------------------------------------"));
}

void Server::SetServerName(const char* sServerName)
{
	const auto oldName = GetServerName();
	strcpy_s(this->sServerName, sServerName);

	if (strcmp(oldName, DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Server Name has been set to %s"), sServerName);
	}
	else
	{
		LOG_DEBUG(CSTRING("Server Name has been changed from %s to %s"), oldName, sServerName);
	}
}

void Server::SetServerPort(const u_short sServerPort)
{
	this->sServerPort = sServerPort;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Server Port has been set to %i"), sServerPort);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Server Port has been set to %i"), GetServerName(), sServerPort);
	}
}

void Server::SetFrequenz(const long long sfrequenz)
{
	this->sfrequenz = sfrequenz;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Refresh-Frequenz has been set to %lld"), sfrequenz);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Refresh-Frequenz has been set to %lld"), GetServerName(), sfrequenz);
	}
}

void Server::SetMaxThreads(const u_short sMaxThreads)
{
	this->sMaxThreads = sMaxThreads;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Max Threads has been set to %i"), sMaxThreads);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Max Threads has been set to %i"), GetServerName(), sMaxThreads);
	}
}

void Server::SetRSAKeySize(const size_t sRSAKeySize)
{
	this->sRSAKeySize = sRSAKeySize;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("RSA Key size has been set to %llu"), sRSAKeySize);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - RSA Key size has been set to %llu"), GetServerName(), sRSAKeySize);
	}
}

void Server::SetAESKeySize(const size_t sAESKeySize)
{
	if (sAESKeySize != CryptoPP::AES::MIN_KEYLENGTH && sAESKeySize != CryptoPP::AES::KEYLENGTH_MULTIPLE && sAESKeySize != CryptoPP::AES::MAX_KEYLENGTH)
	{
		if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
		{
			LOG_ERROR(CSTRING("AES Key size of %llu is not valid!"), sAESKeySize);
		}
		else
		{
			LOG_ERROR(CSTRING("[%s] - AES Key size of %llu is not valid!"), GetServerName(), sAESKeySize);
		}

		return;
	}

	this->sAESKeySize = sAESKeySize;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("AES Key size has been set to %i"), sAESKeySize);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - AES Key size has been set to %i"), GetServerName(), sAESKeySize);
	}
}

void Server::SetCryptPackage(const bool sCryptPackage)
{
	this->sCryptPackage = sCryptPackage;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Crypt Package has been %s"), sCryptPackage ? CSTRING("enabled") : CSTRING("disabled"));
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Crypt Package has been %s"), GetServerName(), sCryptPackage ? CSTRING("enabled") : CSTRING("disabled"));
	}
}

void Server::SetCompressPackage(const bool sCompressPackage)
{
	this->sCompressPackage = sCompressPackage;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Compress Package has been %s"), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Compress Package has been %s"), GetServerName(), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
	}
}

void Server::SetTCPReadTimeout(const long sTCPReadTimeout)
{
	this->sTCPReadTimeout = sTCPReadTimeout;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("TCP Read timeout has been set to %i"), sTCPReadTimeout);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - TCP Read timeout has been set to %i"), GetServerName(), sTCPReadTimeout);
	}
}

void Server::SetCalcLatencyInterval(const long sCalcLatencyInterval)
{
	this->sCalcLatencyInterval = sCalcLatencyInterval;

	if (strcmp(GetServerName(), DEFAULT_SERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Calculate latency interval has been set to %i"), sCalcLatencyInterval);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Calculate latency interval has been set to %i"), GetServerName(), sCalcLatencyInterval);
	}
}

const char* Server::GetServerName() const
{
	return sServerName;
}

u_short Server::GetServerPort() const
{
	return sServerPort;
}

u_short Server::GetMaxThreads() const
{
	return sMaxThreads;
}

long long Server::GetFrequenz() const
{
	return sfrequenz;
}

size_t Server::GetRSAKeySize() const
{
	return sRSAKeySize;
}

size_t Server::GetAESKeySize() const
{
	return sAESKeySize;
}

bool Server::GetCryptPackage() const
{
	return sCryptPackage;
}

bool Server::GetCompressPackage() const
{
	return sCompressPackage;
}

long Server::GetTCPReadTimeout() const
{
	return sTCPReadTimeout;
}

long Server::GetCalcLatencyInterval() const
{
	return sCalcLatencyInterval;
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
	memset(_dataReceive, NULL, DEFAULT_SERVER_MAX_PACKET_SIZE);
}

void Server::network_t::clear()
{
	deallocData();
	_data_size = NULL;
	_data_full_size = NULL;
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
	RSA.GenerateKeys(size, 3);
	setHandshakeStatus(false);
}

void Server::cryption_t::deleteKeyPair()
{
	RSA.DeleteKeys();
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

Server::NET_IPEER Server::InsertPeer(const sockaddr_in client_addr, const SOCKET socket)
{
	// UniqueID is equal to socket, since socket is already an unique ID
	NET_IPEER newPeer;
	newPeer.UniqueID = socket;
	newPeer.pSocket = socket;
	newPeer.client_addr = client_addr;

	/* Set Read Timeout */
	timeval tv ={};
	tv.tv_sec = GetTCPReadTimeout();
	tv.tv_usec = 0;
	setsockopt(newPeer.pSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	IncreasePeersCounter();

	// callback
	OnPeerConnect(newPeer);

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): connected!"), GetServerName(), newPeer.IPAddr().get());
	return newPeer;
}

bool Server::ErasePeer(NET_PEER peer)
{
	if (!peer.isAsync)
	{
		// close endpoint
		if (peer.pSocket)
		{
			closesocket(peer.pSocket);
			peer.pSocket = INVALID_SOCKET;
		}

		// callback
		OnPeerDisconnect(peer);

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): disconnected!"), GetServerName(), peer.IPAddr().get());

		peer.clear();

		DecreasePeersCounter();
		return true;
	}

	// close endpoint
	if (peer.pSocket)
	{
		closesocket(peer.pSocket);
		peer.pSocket = INVALID_SOCKET;
	}

	return true;
}

void Server::UpdatePeer(NET_PEER peer)
{
	if (!peer)
		return;

	// Calculate latency interval
	if (peer.lastCalcLatency < CURRENTCLOCKTIME)
	{
		std::thread(&LatencyThread, peer).detach();
		peer.lastCalcLatency = CREATETIMER(GetCalcLatencyInterval());
	}
}

size_t Server::GetNextPackageSize(NET_PEER peer) const
{
	if (!peer)
		return 0;

	return peer.network.getDataFullSize();
}

size_t Server::GetReceivedPackageSize(NET_PEER peer) const
{
	if (!peer)
		return 0;

	return peer.network.getDataSize();
}

float Server::GetReceivedPackageSizeAsPerc(NET_PEER peer) const
{
	if (!peer)
		return 0.0f;

	// Avoid dividing zero with zero
	if (peer.network.getDataSize() <= 0)
		return 0.0f;

	if (peer.network.getDataFullSize() <= 0)
		return 0;

	auto perc = static_cast<float>(peer.network.getDataSize()) / static_cast<float>(peer.network.getDataFullSize()) * 100;
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
	latency = -1;
	lastCalcLatency = 0;

	network.clear();
	network.reset();

	cryption.deleteKeyPair();
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
	const auto buf = new char[INET_ADDRSTRLEN];
	return IPRef(inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN));
}

void Server::DisconnectPeer(NET_PEER peer, const int code)
{
	if (!peer)
		return;

	Package PKG;
	PKG.Append(CSTRING("code"), code);
	NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_ClosePackage, pkg);

	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): has been disconnected, reason: %s"), GetServerName(), peer.IPAddr().get(), NetGetErrorMessage(code));

	// now after we have sent him the reason, close connection
	ErasePeer(peer);
}

/* Thread functions */
void Server::TickThread()
{
	LOG_DEBUG(CSTRING("TickThread() has been started!"));

	while (IsRunning() && !NeedExit())
	{
		Tick();

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	}

	LOG_DEBUG(CSTRING("TickThread() has been closed!"));
}

void Server::AcceptorThread()
{
	LOG_DEBUG(CSTRING("AcceptorThread() has been started!"));

	while (IsRunning() && !NeedExit())
	{
		Acceptor();

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz() * 2));
	}

	LOG_DEBUG(CSTRING("AcceptorThread() has been closed!"));
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

bool Server::Start(const char* serverName, const u_short serverPort)
{
	if (IsRunning())
		return false;

	/* set name and port */
	SetServerName(serverName);
	SetServerPort(serverPort);

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
		LOG_ERROR(CSTRING("[%s] - WSAStartup failed with error: %d"), GetServerName(), res);
		return false;
	}

	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	const auto Port = std::to_string(GetServerPort());
	res = getaddrinfo(NULLPTR, Port.data(), &hints, &result);

	if (res != 0) {
		LOG_ERROR(CSTRING("[%s] - getaddrinfo failed with error: %d"), GetServerName(), res);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	SetListenSocket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));

	if (GetListenSocket() == INVALID_SOCKET) {
		LOG_ERROR(CSTRING("[%s] - socket failed with error: %ld"), GetServerName(), WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;
	res = ioctlsocket(GetListenSocket(), FIONBIO, &iMode);

	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[%s] - ioctlsocket failed with error: %d"), GetServerName(), WSAGetLastError());
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	res = bind(GetListenSocket(), result->ai_addr, static_cast<int>(result->ai_addrlen));

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - bind failed with error: %d"), GetServerName(), WSAGetLastError());
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
		LOG_ERROR(CSTRING("[%s] - listen failed with error: %d"), GetServerName(), WSAGetLastError());
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// Create all needed Threads
	std::thread(&Server::TickThread, this).detach();
	std::thread(&Server::AcceptorThread, this).detach();

	SetRunning(true);
	LOG_SUCCESS(CSTRING("[%s] - started on Port: %d"), GetServerName(), GetServerPort());
	return true;
}

bool Server::Close()
{
	if (!IsRunning())
	{
		LOG_ERROR(CSTRING("[%s] - Can't close server, because server is not running!"), GetServerName());
		return false;
	}

	SetRunning(false);

	if (GetListenSocket())
		closesocket(GetListenSocket());

	if (GetAcceptSocket())
		closesocket(GetAcceptSocket());

	WSACleanup();

	LOG_DEBUG(CSTRING("[%s] - Closed!"), GetServerName());
	return true;
}

void Server::Terminate()
{
	if (IsRunning())
		Close();

	LOG_DEBUG(CSTRING("[%s] - Terminated!"), GetServerName());
	delete this;
}

bool Server::NeedExit() const
{
	return DoExit;
}

void Server::SingleSend(NET_PEER peer, const char* data, size_t size)
{
	if (!peer)
		return;

	do
	{
		const auto res = send(peer.pSocket, data, static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}
		if (res < 0)
			break;

		size -= res;
	} while (size > 0);
}

void Server::SingleSend(NET_PEER peer, BYTE*& data, size_t size)
{
	if (!peer)
	{
		FREE(data);
		return;
	}

	do
	{
		const auto res = send(peer.pSocket, reinterpret_cast<const char*>(data), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				FREE(data);
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), GetServerName(), peer.IPAddr().get());
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

void Server::SingleSend(NET_PEER peer, CPOINTER<BYTE>& data, size_t size)
{
	if (!peer)
	{
		data.free();
		return;
	}

	do
	{
		const auto res = send(peer.pSocket, reinterpret_cast<const char*>(data.get()), static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEACCES:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOBUFS:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEHOSTUNREACH:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), GetServerName(), peer.IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				data.free();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), GetServerName(), peer.IPAddr().get());
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
	if (!peer)
		return;

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
	if (GetCryptPackage() && peer.cryption.getHandshakeStatus())
	{
		NET_AES aes;

		/* Generate new AES Keypair */
		CPOINTER<BYTE> Key(ALLOC<BYTE>(GetAESKeySize() + 1));
		Random::GetRandStringNew(Key.reference().get(), GetAESKeySize());
		Key.get()[GetAESKeySize()] = '\0';

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

		NET_RSA rsa;
		if (!rsa.Init(peer.cryption.RSA.PublicKey().get(), (char*)CSTRING("")))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed to Init RSA [0]"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitRSA);
			return;
		}

		/* Encrypt AES Keypair using RSA */
		auto KeySize = GetAESKeySize();
		if (!rsa.encryptBase64(Key.reference().ref(), KeySize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed Key to encrypt and encode to base64"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_CryptKeyBase64);
			return;
		}

		size_t IVSize = CryptoPP::AES::BLOCKSIZE;
		if (!rsa.encryptBase64(IV.reference().ref(), IVSize))
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

		combinedSize = dataBufferSize + sizeof(NET_PACKAGE_HEADER) - 1 + sizeof(NET_PACKAGE_SIZE) - 1 + sizeof(NET_DATA) - 1 + sizeof(NET_PACKAGE_FOOTER) - 1 + sizeof(NET_AES_KEY) - 1 + sizeof(NET_AES_IV) - 1 + KeySize + IVSize + 8;

		// Append Raw data package size
		if (PKG.HasRawData())
		{
			if (GetCompressPackage())
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.reference(), data.size());
			}

			combinedSize += PKG.GetRawDataFullSize();
		}

		std::string dataSizeStr;
		if (GetCompressPackage())
		{
			dataSizeStr = std::to_string(dataBufferSize);
			combinedSize += dataSizeStr.length();
		}
		else
		{
			dataSizeStr = std::to_string(buffer.GetSize());
			combinedSize += dataSizeStr.length();
		}

		const auto KeySizeStr = std::to_string(KeySize);
		combinedSize += KeySizeStr.length();

		const auto IVSizeStr = std::to_string(IVSize);
		combinedSize += IVSizeStr.length();

		const auto EntirePackageSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

		/* Append Package Header */
		SingleSend(peer, NET_PACKAGE_HEADER, sizeof(NET_PACKAGE_HEADER) - 1);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, sizeof(NET_PACKAGE_SIZE) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);

		/* Append Package Key */
		SingleSend(peer, NET_AES_KEY, sizeof(NET_AES_KEY) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);
		SingleSend(peer, KeySizeStr.data(), KeySizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
		SingleSend(peer, Key, KeySize);

		/* Append Package IV */
		SingleSend(peer, NET_AES_IV, sizeof(NET_AES_IV) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);
		SingleSend(peer, IVSizeStr.data(), IVSizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
		SingleSend(peer, IV, IVSize);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, sizeof(NET_RAW_DATA_KEY) - 1);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length());
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
				SingleSend(peer, data.key(), data.keylength() + 1);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, sizeof(NET_RAW_DATA) - 1);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length());
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
				SingleSend(peer, data.value(), data.size());
				PKG.DoNotDestruct();
			}
		}

		SingleSend(peer, NET_DATA, sizeof(NET_DATA) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
		SingleSend(peer, dataBuffer, dataBufferSize);

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, sizeof(NET_PACKAGE_FOOTER) - 1);
	}
	else
	{
		CPOINTER<BYTE> dataBuffer;
		size_t dataBufferSize = NULL;
		if (GetCompressPackage())
		{
			dataBufferSize = buffer.GetSize();
			dataBuffer = ALLOC<BYTE>(dataBufferSize + 1);
			memcpy(dataBuffer.get(), buffer.GetString(), dataBufferSize);
			buffer.Flush();
			dataBuffer.get()[dataBufferSize] = '\0';

			CompressData(dataBuffer.reference().get(), dataBufferSize);

			combinedSize = dataBufferSize + sizeof(NET_PACKAGE_HEADER) - 1 + sizeof(NET_PACKAGE_SIZE) - 1 + sizeof(NET_DATA) - 1 + sizeof(NET_PACKAGE_FOOTER) - 1 + 4;
		}
		else
			combinedSize = buffer.GetSize() + sizeof(NET_PACKAGE_HEADER) - 1 + sizeof(NET_PACKAGE_SIZE) - 1 + sizeof(NET_DATA) - 1 + sizeof(NET_PACKAGE_FOOTER) - 1 + 4;

		// Append Raw data package size
		if (PKG.HasRawData())
		{
			if (GetCompressPackage())
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.reference(), data.size());
			}

			combinedSize += PKG.GetRawDataFullSize();
		}

		std::string dataSizeStr;
		if (GetCompressPackage())
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

		/* Append Package Header */
		SingleSend(peer, NET_PACKAGE_HEADER, sizeof(NET_PACKAGE_HEADER) - 1);

		// Append Package Size Syntax
		SingleSend(peer, NET_PACKAGE_SIZE, sizeof(NET_PACKAGE_SIZE) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);
		SingleSend(peer, EntirePackageSizeStr.data(), EntirePackageSizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);

		/* Append Package Data */
		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto data : rawData)
			{
				// Append Key
				SingleSend(peer, NET_RAW_DATA_KEY, sizeof(NET_RAW_DATA_KEY) - 1);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);

				const auto KeyLengthStr = std::to_string(data.keylength() + 1);

				SingleSend(peer, KeyLengthStr.data(), KeyLengthStr.length());
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
				SingleSend(peer, data.key(), data.keylength() + 1);

				// Append Raw Data
				SingleSend(peer, NET_RAW_DATA, sizeof(NET_RAW_DATA) - 1);
				SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, 1);

				const auto rawDataLengthStr = std::to_string(data.size());

				SingleSend(peer, rawDataLengthStr.data(), rawDataLengthStr.length());
				SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);
				SingleSend(peer, data.value(), data.size());
				PKG.DoNotDestruct();
			}
		}

		SingleSend(peer, NET_DATA, sizeof(NET_DATA) - 1);
		SingleSend(peer, NET_PACKAGE_BRACKET_OPEN, sizeof(NET_PACKAGE_BRACKET_OPEN) - 1);
		SingleSend(peer, dataSizeStr.data(), dataSizeStr.length());
		SingleSend(peer, NET_PACKAGE_BRACKET_CLOSE, 1);

		if (GetCompressPackage())
			SingleSend(peer, dataBuffer, dataBufferSize);
		else
			SingleSend(peer, buffer.GetString(), buffer.GetSize());

		/* Append Package Footer */
		SingleSend(peer, NET_PACKAGE_FOOTER, sizeof(NET_PACKAGE_FOOTER) - 1);
	}
}

short Server::Handshake(NET_PEER peer)
{
	if (!peer)
		return ServerHandshake::peer_not_valid;

	const auto data_size = recv(peer.pSocket, reinterpret_cast<char*>(peer.network.getDataReceive()), DEFAULT_SERVER_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENETDOWN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEFAULT:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENOTCONN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINTR:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINPROGRESS:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENETRESET:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAENOTSOCK:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEOPNOTSUPP:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAESHUTDOWN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEWOULDBLOCK:
			peer.network.reset();
			return ServerHandshake::would_block;

		case WSAEMSGSIZE:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAEINVAL:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAECONNABORTED:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAETIMEDOUT:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		case WSAECONNRESET:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;

		default:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return ServerHandshake::error;
		}
	}
	if (data_size == 0)
	{
		peer.network.reset();
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), GetServerName(), peer.IPAddr().get());
		ErasePeer(peer);
		return ServerHandshake::error;
	}
	peer.network.getDataReceive()[data_size] = '\0';

	if (!peer.network.dataValid())
	{
		peer.network.allocData(data_size);
		memcpy(peer.network.getData(), peer.network.getDataReceive(), data_size);
	}
	else
	{
		//* store incomming */
		const auto newBuffer = ALLOC<BYTE>(peer.network.getDataSize() + data_size + 1);
		memcpy(newBuffer, peer.network.getData(), peer.network.getDataSize());
		memcpy(&newBuffer[peer.network.getDataSize()], peer.network.getDataReceive(), data_size);
		peer.network.setDataSize(peer.network.getDataSize() + data_size);
		peer.network.setData(newBuffer); // pointer swap
	}

	peer.network.reset();

	std::string tmp(reinterpret_cast<const char*>(peer.network.getData()));
	const auto pos = tmp.find(CSTRING("GET / HTTP/1.1"));
	if (pos != std::string::npos)
	{
		std::map<std::string, std::string> entries;

		std::istringstream s(reinterpret_cast<const char*>(peer.network.getData()));
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
		peer.network.clear();

		const auto stringUpdate = CSTRING("Upgrade");

		/* check if we have connected through a browser client */
		if (strcmp(entries[stringUpdate].data(), CSTRING("websocket")) != 0)
			return ServerHandshake::is_websocket;

		return ServerHandshake::is_not_websocket;
	}

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen on Handshake"), GetServerName(), peer.IPAddr().get());

	// clear data
	peer.network.clear();
	return ServerHandshake::error;
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
		// disable nagle on the client's socket
		char value = 1;
		setsockopt(GetAcceptSocket(), IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

		std::thread(&Server::ReceiveThread, this, client_addr, GetAcceptSocket()).detach();
	}
}

void Server::ReceiveThread(const sockaddr_in client_addr, const SOCKET socket)
{
	auto peer = InsertPeer(client_addr, socket);

	do
	{
		peer.setAsync(true);
		const auto res = Handshake(peer);
		if (res == ServerHandshake::peer_not_valid)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to invalid socket!"), GetServerName(), peer.IPAddr().get());

			// erase him
			peer.setAsync(false);
			ErasePeer(peer);
			return;
		}
		if (res == ServerHandshake::would_block)
		{
			peer.setAsync(false);
			break;
		}
		if (res == ServerHandshake::is_websocket)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to websocket!"), GetServerName(), peer.IPAddr().get());

			// erase him
			peer.setAsync(false);
			ErasePeer(peer);
			return;
		}
		if (res == ServerHandshake::error)
		{
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake error!"), GetServerName(), peer.IPAddr().get());

			peer.setAsync(false);
			ErasePeer(peer);
			return;
		}
		if (res == ServerHandshake::is_not_websocket)
		{
			peer.setAsync(false);
			break;
		}
		peer.setAsync(false);
	} while (!DoExit);

	if (GetCryptPackage())
	{
		/* Create new RSA Key Pair */
		peer.cryption.createKeyPair(GetRSAKeySize());

		Package PKG;
		PKG.Append<const char*>(CSTRING("PublicKey"), peer.cryption.RSA.PublicKey().get());
		NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake, pkg);
	}
	else
	{
		// keep it empty, we get it filled back
		Package PKG;
		NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage, pkg);
	}

	while (peer)
	{
		if (!IsRunning() || NeedExit())
			break;

		UpdatePeer(peer);
		OnPeerUpdate(peer);

		if (peer.pSocket)
		{
			peer.setAsync(true);
			DoReceive(peer);
			peer.setAsync(false);
		}
		else
		{
			peer.setAsync(false);
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	}

	// erase him
	peer.setAsync(false);
	ErasePeer(peer);
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
void Server::DoReceive(NET_PEER peer)
{
	if (!peer || !peer.pSocket)
		return;

	const auto data_size = recv(peer.pSocket, reinterpret_cast<char*>(peer.network.getDataReceive()), DEFAULT_SERVER_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAENETDOWN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEFAULT:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAENOTCONN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEINTR:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEINPROGRESS:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAENETRESET:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAENOTSOCK:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEOPNOTSUPP:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAESHUTDOWN:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEWOULDBLOCK:
			ProcessPackages(peer);
			peer.network.reset();
			return;

		case WSAEMSGSIZE:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAEINVAL:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;


		case WSAECONNABORTED:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAETIMEDOUT:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		case WSAECONNRESET:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;

		default:
			peer.network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer.IPAddr().get());
			ErasePeer(peer);
			return;
		}
	}
	if (data_size == 0)
	{
		peer.network.reset();
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), GetServerName(), peer.IPAddr().get());
		ErasePeer(peer);
		return;
	}

	if (!peer.network.dataValid())
	{
		// check if incomming is even valid
		if (memcmp(&peer.network.getDataReceive()[0], NET_PACKAGE_BRACKET_OPEN, 1) != 0)
		{
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedPackage);
			peer.network.reset();
			return;
		}
		if (memcmp(&peer.network.getDataReceive()[0], NET_PACKAGE_HEADER, sizeof(NET_PACKAGE_HEADER) - 1) != 0)
		{
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedPackage);
			peer.network.reset();
			return;
		}

		peer.network.allocData(data_size);
		memcpy(peer.network.getData(), peer.network.getDataReceive(), data_size);
	}
	else
	{
		if (peer.network.getDataFullSize() > 0)
		{
			if (peer.network.getDataSize() + data_size > peer.network.getDataFullSize())
			{
				peer.network.setDataFullSize(peer.network.getDataFullSize() + data_size);

				/* store incomming */
				const auto newBuffer = ALLOC<BYTE>(peer.network.getDataFullSize() + 1);
				memcpy(newBuffer, peer.network.getData(), peer.network.getDataSize());
				memcpy(&newBuffer[peer.network.getDataSize()], peer.network.getDataReceive(), data_size);
				peer.network.setDataSize(peer.network.getDataSize() + data_size);
				peer.network.setData(newBuffer); // pointer swap
			}
			else
			{
				memcpy(&peer.network.getData()[peer.network.getDataSize()], peer.network.getDataReceive(), data_size);
				peer.network.setDataSize(peer.network.getDataSize() + data_size);
			}
		}
		else
		{
			//* store incomming */
			const auto newBuffer = ALLOC<BYTE>(peer.network.getDataSize() + data_size + 1);
			memcpy(newBuffer, peer.network.getData(), peer.network.getDataSize());
			memcpy(&newBuffer[peer.network.getDataSize()], peer.network.getDataReceive(), data_size);
			peer.network.setDataSize(peer.network.getDataSize() + data_size);
			peer.network.setData(newBuffer); // pointer swap
		}
	}

	// check if we have something to process already - since we have a fixed data receive size, should this be fine to process in everytick
	for (size_t it = DEFAULT_SERVER_MAX_PACKET_SIZE - 1; it > 0; --it)
	{
		if (!memcmp(&peer.network.getDataReceive()[it], NET_PACKAGE_BRACKET_OPEN, 1))
		{
			if (!memcmp(&peer.network.getDataReceive()[it], NET_PACKAGE_FOOTER, sizeof(NET_PACKAGE_FOOTER) - 1))
			{
				ProcessPackages(peer);
				peer.network.reset();
				return;
			}
		}
	}

	peer.network.reset();
	GetPackageDataSize(peer);
}

void Server::GetPackageDataSize(NET_PEER peer) const
{
	if (!peer.network.getDataSize()
		|| peer.network.getDataSize() == INVALID_SIZE)
		return;

	if (peer.network.getDataFullSize() > 0)
		return;

	for (size_t it = 0; it < peer.network.getDataSize(); ++it)
	{
		if (!memcmp(&peer.network.getData()[it], NET_PACKAGE_BRACKET_OPEN, 1))
		{
			// find data full size
			if (!memcmp(&peer.network.getData()[it], NET_PACKAGE_SIZE, sizeof(NET_PACKAGE_SIZE) - 1))
			{
				const auto startPos = it + sizeof(NET_PACKAGE_SIZE) - 1;
				size_t endPos = NULL;
				for (auto z = startPos; z < peer.network.getDataSize(); ++z)
				{
					if (!memcmp(&peer.network.getData()[z], NET_PACKAGE_BRACKET_CLOSE, 1))
					{
						endPos = z;
						break;
					}
				}

				if (!endPos)
					return;

				const auto size = endPos - startPos - 1;
				CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(size + 1));
				memcpy(dataSizeStr.get(), &peer.network.getData()[startPos + 1], size);
				dataSizeStr.get()[size] = '\0';
				const auto dataSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
				peer.network.setDataFullSize(dataSize);
				dataSizeStr.free();

				const auto newBuffer = ALLOC<BYTE>(peer.network.getDataFullSize() + 1);
				memcpy(newBuffer, peer.network.getData(), peer.network.getDataSize());
				newBuffer[peer.network.getDataFullSize()] = '\0';
				peer.network.setData(newBuffer); // pointer swap

				break;
			}
		}
	}
}

void Server::ProcessPackages(NET_PEER peer)
{
	if (!peer.network.getDataSize()
		|| peer.network.getDataSize() == INVALID_SIZE)
		return;

	do
	{
		auto continuePackage = false;

		// search for footer
		for (auto it = peer.network.getDataSize() - 1; it > 0; --it)
		{
			if (!memcmp(&peer.network.getData()[it], NET_PACKAGE_BRACKET_OPEN, 1))
			{
				if (!memcmp(&peer.network.getData()[it], NET_PACKAGE_FOOTER, sizeof(NET_PACKAGE_FOOTER) - 1))
				{
					// check if we have a header
					auto idx = NULL;
					if (memcmp(&peer.network.getData()[0], NET_PACKAGE_HEADER, sizeof(NET_PACKAGE_HEADER) - 1) != 0)
					{
						LOG_ERROR(CSTRING("Package has no header"));
						peer.network.clear();
						return;
					}

					idx += sizeof(NET_PACKAGE_HEADER) - 1 + sizeof(NET_PACKAGE_SIZE) - 1;

					// read entire Package size
					size_t entirePackageSize = NULL;
					size_t offsetBegin = NULL;
					{
						for (size_t y = idx; y < peer.network.getDataSize(); ++y)
						{
							if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								offsetBegin = y;
								const auto size = y - idx - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(size + 1));
								memcpy(dataSizeStr.get(), &peer.network.getData()[idx + 1], size);
								dataSizeStr.get()[size] = '\0';
								entirePackageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();
								break;
							}
						}
					}

					// Execute the package
					ExecutePackage(peer, entirePackageSize, offsetBegin);

					// re-alloc buffer
					const auto leftSize = static_cast<int>(peer.network.getDataSize() - entirePackageSize) > 0 ? peer.network.getDataSize() - entirePackageSize : INVALID_SIZE;
					if (leftSize != INVALID_SIZE
						&& leftSize > 0)
					{
						const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
						memcpy(leftBuffer, &peer.network.getData()[entirePackageSize], leftSize);
						leftBuffer[leftSize] = '\0';
						peer.network.setData(leftBuffer); // swap pointer
						peer.network.setDataSize(leftSize);
						peer.network.setDataFullSize(NULL);

						continuePackage = true;
					}
					else
						peer.network.clear();

					break;
				}
			}
		}

		if (!continuePackage)
			break;

	} while (true);
}

void Server::ExecutePackage(NET_PEER peer, const size_t size, const size_t begin)
{
	if (memcmp(&peer.network.getData()[size - sizeof(NET_PACKAGE_FOOTER) + 1], NET_PACKAGE_FOOTER, sizeof(NET_PACKAGE_FOOTER) - 1) != 0)
	{
		LOG_ERROR(CSTRING("Package has no footer"));
		peer.network.clear();
		return;
	}

	CPOINTER<BYTE> data;
	std::vector<Package_RawData_t> rawData;

	/* Crypt */
	if (GetCryptPackage() && peer.cryption.getHandshakeStatus())
	{
		auto offset = begin + 1;

		CPOINTER<BYTE> AESKey;
		size_t AESKeySize;

		// look for key tag
		if (!memcmp(&peer.network.getData()[offset], NET_AES_KEY, sizeof(NET_AES_KEY) - 1))
		{
			offset += sizeof(NET_AES_KEY) - 1;

			// read size
			for (auto y = offset; y < peer.network.getDataSize(); ++y)
			{
				if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESKeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESKey = ALLOC<BYTE>(AESKeySize + 1);
			memcpy(AESKey.get(), &peer.network.getData()[offset], AESKeySize);
			AESKey.get()[AESKeySize] = '\0';

			offset += AESKeySize;
		}

		CPOINTER<BYTE> AESIV;
		size_t AESIVSize;

		// look for iv tag
		if (!memcmp(&peer.network.getData()[offset], NET_AES_IV, sizeof(NET_AES_IV) - 1))
		{
			offset += sizeof(NET_AES_IV) - 1;

			// read size
			for (auto y = offset; y < peer.network.getDataSize(); ++y)
			{
				if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
				{
					const auto psize = y - offset - 1;
					CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
					memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
					dataSizeStr.get()[psize] = '\0';
					AESIVSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
					dataSizeStr.free();

					offset += psize + 2;
					break;
				}
			}

			// read the data
			AESIV = ALLOC<BYTE>(AESIVSize + 1);
			memcpy(AESIV.get(), &peer.network.getData()[offset], AESIVSize);
			AESIV.get()[AESIVSize] = '\0';

			offset += AESIVSize;
		}

		// Init RSA
		NET_RSA rsa;
		if (!rsa.Init((char*)CSTRING(""), peer.cryption.RSA.PrivateKey().get()))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed to Init RSA [1]"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitRSA);
			return;
		}

		if (!rsa.decryptBase64(AESKey.reference().ref(), AESKeySize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed Key to decrypt and decode the base64"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptKeyBase64);
			return;
		}

		if (!rsa.decryptBase64(AESIV.reference().ref(), AESIVSize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed IV to decrypt and decode the base64"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DecryptIVBase64);
			return;
		}

		NET_AES aes;
		if (!aes.Init(reinterpret_cast<char*>(AESKey.get()), reinterpret_cast<char*>(AESIV.get())))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed to Init AES [1]"));
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_InitAES);
			return;
		}

		AESKey.free();
		AESIV.free();

		do
		{
			// look for raw data tag
			if (!memcmp(&peer.network.getData()[offset], NET_RAW_DATA_KEY, sizeof(NET_RAW_DATA_KEY) - 1))
			{
				offset += sizeof(NET_RAW_DATA_KEY) - 1;

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < peer.network.getDataSize(); ++y)
					{
						if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &peer.network.getData()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&peer.network.getData()[offset], NET_RAW_DATA, sizeof(NET_RAW_DATA) - 1))
				{
					offset += sizeof(NET_RAW_DATA) - 1;

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < peer.network.getDataSize(); ++y)
						{
							if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &peer.network.getData()[offset], packageSize };

					if (GetCompressPackage())
						DecompressData(entry.reference(), entry.size());

					/* decrypt aes */
					if (!aes.decrypt(entry.value(), entry.size()))
					{
						LOG_PEER(CSTRING("Failure on decrypting buffer"));
						return;
					}

					rawData.emplace_back(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&peer.network.getData()[offset], NET_DATA, sizeof(NET_DATA) - 1))
			{
				offset += sizeof(NET_DATA) - 1;

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < peer.network.getDataSize(); ++y)
					{
						if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
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
				memcpy(data.get(), &peer.network.getData()[offset], packageSize);
				data.get()[packageSize] = '\0';

				offset += packageSize;

				if (GetCompressPackage())
					DecompressData(data.reference().get(), packageSize);

				/* decrypt aes */
				if (!aes.decrypt(data.get(), packageSize))
				{
					LOG_PEER(CSTRING("Failure on decrypting buffer"));
					data.free();
					return;
				}
			}

			// we have reached the end of reading
			if (offset + sizeof(NET_PACKAGE_FOOTER) - 1 == size)
				break;

		} while (true);
	}
	else
	{
		auto offset = begin + 1;

		do
		{
			// look for raw data tag
			if (!memcmp(&peer.network.getData()[offset], NET_RAW_DATA_KEY, sizeof(NET_RAW_DATA_KEY) - 1))
			{
				offset += sizeof(NET_RAW_DATA_KEY) - 1;

				// read size
				CPOINTER<BYTE> key;
				size_t KeySize = NULL;
				{
					for (auto y = offset; y < peer.network.getDataSize(); ++y)
					{
						if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					key = ALLOC<BYTE>(KeySize + 1);
					memcpy(key.get(), &peer.network.getData()[offset], KeySize);
					key.get()[KeySize] = '\0';

					offset += KeySize;
				}

				if (!memcmp(&peer.network.getData()[offset], NET_RAW_DATA, sizeof(NET_RAW_DATA) - 1))
				{
					offset += sizeof(NET_RAW_DATA) - 1;

					// read size
					size_t packageSize = NULL;
					{
						for (auto y = offset; y < peer.network.getDataSize(); ++y)
						{
							if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								const auto psize = y - offset - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
								memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
								dataSizeStr.get()[psize] = '\0';
								packageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();

								offset += psize + 2;
								break;
							}
						}
					}

					Package_RawData_t entry = { (char*)key.get(), &peer.network.getData()[offset], packageSize };

					if (GetCompressPackage())
						DecompressData(entry.reference(), entry.size());

					rawData.emplace_back(entry);
					key.free();

					offset += packageSize;
				}
			}

			// look for data tag
			if (!memcmp(&peer.network.getData()[offset], NET_DATA, sizeof(NET_DATA) - 1))
			{
				offset += sizeof(NET_DATA) - 1;

				// read size
				size_t packageSize = NULL;
				{
					for (auto y = offset; y < peer.network.getDataSize(); ++y)
					{
						if (!memcmp(&peer.network.getData()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &peer.network.getData()[offset + 1], psize);
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
				memcpy(data.get(), &peer.network.getData()[offset], packageSize);
				data.get()[packageSize] = '\0';

				offset += packageSize;

				if (GetCompressPackage())
					DecompressData(data.reference().get(), packageSize);
			}

			// we have reached the end of reading
			if (offset + sizeof(NET_PACKAGE_FOOTER) - 1 == size)
				break;

		} while (true);
	}

	if (!data.valid())
	{
		LOG_PEER(CSTRING("Data is nullptr"));
		return;
	}

	Package PKG;
	PKG.Parse(reinterpret_cast<char*>(data.get()));
	if (!PKG.GetPackage().HasMember(CSTRING("ID")))
	{
		LOG_PEER(CSTRING("Package ID is invalid!"));
		data.free();
		return;
	}

	const auto id = PKG.GetPackage().FindMember(CSTRING("ID"))->value.GetInt();
	if (id < 0)
	{
		LOG_PEER(CSTRING("Package ID is invalid!"));
		data.free();
		return;
	}

	if (!PKG.GetPackage().HasMember(CSTRING("CONTENT")))
	{
		LOG_PEER(CSTRING("Package Content is invalid!"));
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
	{
		if (!CheckData(peer, id, Content))
		{
			LOG_PEER(CSTRING("Package is not defined!"));
		}
	}

	data.free();
}

void Server::CompressData(BYTE** data, size_t& size) const
{
	/* Compression */
	if (GetCompressPackage())
	{
#ifdef DEBUG
		const auto PrevSize = size;
#endif
		const NET_ZLIB compress;
		compress.Compress(data, size);
#ifdef DEBUG
		LOG_DEBUG(CSTRING("Compressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

void Server::DecompressData(BYTE** data, size_t& size) const
{
	/* Compression */
	if (GetCompressPackage())
	{
		auto copy = ALLOC<BYTE>(size + 1);
		memcpy(copy, *data, size);
		copy[size] = '\0';

#ifdef DEBUG
		const auto PrevSize = size;
#endif
		const NET_ZLIB compress;
		compress.Decompress(&copy, size);

		LOG("%s", copy);

		data[0] = copy; // swap pointer;
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
if (peer.estabilished)
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has already been estabilished, something went wrong!"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
	return;
}
if (peer.cryption.getHandshakeStatus())
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has already done the RSA Handshake, something went wrong!"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
	return;
}

const auto publicKey = PKG.String(CSTRING("PublicKey"));

if (!publicKey.valid()) // empty
{
	LOG_ERROR(CSTRING("[%s][%s] - Weird, Peer ('%s'): has sent an empty RSA Public Key!"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_EmptyRSAPublicKey);
	return;
}

// overwrite stored Public Key with new from Server
const auto size = strlen(publicKey.value());
const auto PublicKey = ALLOC<char>(size + 1);
memcpy(PublicKey, publicKey.value(), size);
PublicKey[size] = '\0';
peer.cryption.RSA.SetPublicKey(PublicKey);

// from now we use the Cryption, synced with Server
peer.cryption.setHandshakeStatus(true);

// RSA Handshake has been finished, keep going with normal process
LOG_PEER(CSTRING("[%s][%s] - RSA Key Handshake were successfully with Peer ('%s')"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());

// keep it empty, we get it filled back
Package pkgVersionsCheck;
NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage, pkgVersionsCheck);
NET_END_FNC_PKG

NET_BEGIN_FNC_PKG(Server, VersionPackage)
// should not happen
if (peer.estabilished)
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has already been estabilished, something went wrong!"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
	return;
}
if (GetCryptPackage() && !peer.cryption.getHandshakeStatus())
{
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has not done the RSA Handshake yet, something went wrong!"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
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
	LOG_ERROR(CSTRING("[%s][%s] - Peer ('%s'): has sent an invalid versions package"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Versionmismatch);
	return;
}

if ((majorVersion.value() == NET_MAJOR_VERSION())
	&& (minorVersion.value() == NET_MINOR_VERSION())
	&& (revision.value() == NET_REVISION())
	&& strcmp(key.value(), NET_KEY()) == 0)
{
	peer.NetVersionMatched = true;

	Package estabilish;
	NET_SEND(peer, NET_NATIVE_PACKAGE_ID::PKG_EstabilishPackage, estabilish);

	peer.estabilished = true;

	LOG_PEER(CSTRING("[%s][%s] - Peer ('%s'): has been estabilished"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get());

	// callback
	OnPeerEstabilished(peer);
}
else
{
	LOG_PEER(CSTRING("[%s][%s] - Peer ('%s'): has sent different Net-Version:\n%i.%i.%i-%s"), GetServerName(), FUNCTION_NAME, peer.IPAddr().get(), majorVersion.value(), minorVersion.value(), revision.value(), key.value());

	// version or key missmatch, disconnect peer
	DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_Versionmismatch);
}
NET_END_FNC_PKG

size_t Server::getCountPeers() const
{
	return _CounterPeersTable;
}
NET_NAMESPACE_END
NET_NAMESPACE_END