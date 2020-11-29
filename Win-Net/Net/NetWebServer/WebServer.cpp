#include "WebServer.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(WebServer)
static void LatencyThread(Server::NET_PEER peer)
{
	if (!peer)
		return;

	peer->bLatency = true;

	const auto ip = peer->IPAddr();
	ICMP _icmp(ip.get());
	_icmp.execute();

	peer->latency = _icmp.getLatency();
	peer->bLatency = false;
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
	Net::Codes::NetLoadErrorCodes();
	SetAllToDefault();
}

Server::~Server()
{
	this->hOrigin.free();
}

void Server::SetAllToDefault()
{
	strcpy_s(sServerName, DEFAULT_WEBSERVER_SERVERNAME);
	sServerPort = DEFAULT_WEBSERVER_SERVERPORT;
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);
	sfrequenz = DEFAULT_WEBSERVER_FREQUENZ;
	DoExit = NULL;
	sTimeSpamProtection = DEFAULT_WEBSERVER_SPAM_PROTECTION_TIMER;
	sMaxPeers = DEFAULT_WEBSERVER_MAX_PEERS;
	sSSL = DEFAULT_WEBSERVER_SSL;
	ctx = nullptr;
	strcpy_s(sCertFileName, DEFAULT_WEBSERVER_CertFileName);
	strcpy_s(sKeyFileName, DEFAULT_WEBSERVER_KeyFileName);
	strcpy_s(sCaFileName, DEFAULT_WEBSERVER_CaFileName);

	sMaxThreads = DEFAULT_WEBSERVER_MAX_THREADS;
	hUseCustom = DEFAULT_WEBSERVER_CustomHandshake;
	hOrigin = nullptr;
	sCompressPackage = DEFAULT_WEBSERVER_COMPRESS_PACKAGES;
	sTCPReadTimeout = DEFAULT_WEBSERVER_TCP_READ_TIMEOUT;
	bWithoutHandshake = DEFAULT_WEBSERVER_WITHOUT_HANDSHAKE;
	sCalcLatencyInterval = DEFAULT_WEBSERVER_CALC_LATENCY_INTERVAL;

	SetRunning(false);

	LOG_DEBUG(CSTRING("---------- SERVER DEFAULT SETTINGS ----------"));
	LOG_DEBUG(CSTRING("Refresh-Frequenz has been set to default value of %lld"), sfrequenz);
	LOG_DEBUG(CSTRING("Spam-Protection timer has been set to default value of %.2f"), sTimeSpamProtection);
	LOG_DEBUG(CSTRING("Max Peers has been set to default value of %i"), sMaxPeers);
	LOG_DEBUG(CSTRING("SSL has been set to default value of %s"), sSSL ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Certification Filename has been set to default value of %s"), sCertFileName);
	LOG_DEBUG(CSTRING("Key Filename has been set to default value of %s"), sKeyFileName);
	LOG_DEBUG(CSTRING("CA Filename has been set to default value of %s"), sCaFileName);
	LOG_DEBUG(CSTRING("Use Custom Handshake has been set to default value of %s"), hUseCustom ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Max Threads has been set to default value of %i"), sMaxThreads);
	LOG_DEBUG(CSTRING("Compress Package has been set to default value of %s"), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("TCP Read timeout has been set to default value of %i"), sTCPReadTimeout);
	LOG_DEBUG(CSTRING("Without Handshake has been set to default value of %s"), bWithoutHandshake ? "TRUE" : "FALSE");
	LOG_DEBUG(CSTRING("Calculate latency interval has been set to default value of %i"), sCalcLatencyInterval);
	LOG_DEBUG(CSTRING("---------------------------------------------"));
}

void Server::SetServerName(const char* sServerName)
{
	const auto oldName = GetServerName();
	strcpy_s(this->sServerName, sServerName);

	if (strcmp(oldName, DEFAULT_WEBSERVER_SERVERNAME) == 0)
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

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
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

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
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

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Max Threads has been set to %i"), sMaxThreads);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Max Threads has been set to %i"), GetServerName(), sMaxThreads);
	}
}

void Server::SetTimeSpamProtection(const float sTimeSpamProtection)
{
	this->sTimeSpamProtection = sTimeSpamProtection;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Spam-Protection timer has been set to %.2f"), sTimeSpamProtection);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Spam-Protection timer has been set to %.2f"), GetServerName(), sTimeSpamProtection);
	}
}

void Server::SetMaxPeers(const unsigned int sMaxPeers)
{
	this->sMaxPeers = sMaxPeers;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Max Peers has been set to %i"), sMaxPeers);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Max Peers has been set to %i"), GetServerName(), sMaxPeers);
	}
}

void Server::SetSSL(const bool sSSL)
{
	this->sSSL = sSSL;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("SSL has been %s"), sSSL ? CSTRING("enabled") : CSTRING("disabled"));
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - SSL has been %s"), GetServerName(), sSSL ? CSTRING("enabled") : CSTRING("disabled"));
	}
}

void Server::SetCertFileName(const char* sCertFileName)
{
	strcpy_s(this->sCertFileName, sCertFileName);

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Certification Filename has been set to %s"), sCertFileName);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Certification Filename has been set to %s"), GetServerName(), sCertFileName);
	}
}

void Server::SetKeyFileName(const char* sKeyFileName)
{
	strcpy_s(this->sKeyFileName, sKeyFileName);

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Key Filename has been set to %s"), sKeyFileName);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Key Filename has been set to %s"), GetServerName(), sKeyFileName);
	}
}

void Server::SetCaFileName(const char* sCaFileName)
{
	strcpy_s(this->sCaFileName, sCaFileName);

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("CA Filename has been set to %s"), sCaFileName);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - CA Filename has been set to %s"), GetServerName(), sCaFileName);
	}
}

void Server::SetCustomHandshakeMethode(const bool hUseCustom)
{
	this->hUseCustom = hUseCustom;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Custom Handshake has been %s"), hUseCustom ? CSTRING("enabled") : CSTRING("disabled"));
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Custom Handshake has been %s"), GetServerName(), hUseCustom ? CSTRING("enabled") : CSTRING("disabled"));
	}
}

void Server::SetHandshakeOriginCompare(const char* hOrigin)
{
	const auto size = strlen(hOrigin);
	this->hOrigin.free();
	this->hOrigin = ALLOC<char>(size + 1);
	memcpy(this->hOrigin.get(), hOrigin, size);
	this->hOrigin.get()[size] = '\0';

	SetCustomHandshakeMethode(true);

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Origin Handshake Variable Check has been set to %s"), hOrigin);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Origin Handshake Variable Check has been set to %s"), GetServerName(), hOrigin);
	}
}

void Server::SetCompressPackage(const bool sCompressPackage)
{
	this->sCompressPackage = sCompressPackage;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
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

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("TCP Read timeout has been set to %i"), GetServerName(), sTCPReadTimeout);
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - TCP Read timeout has been set to %i"), GetServerName(), sTCPReadTimeout);
	}
}


void Server::SetWithoutHandshake(const bool bWithoutHandshake)
{
	this->bWithoutHandshake = bWithoutHandshake;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
	{
		LOG_DEBUG(CSTRING("Without Handshake has been set to %s"), GetServerName(), bWithoutHandshake ? "TRUE" : "FALSE");
	}
	else
	{
		LOG_DEBUG(CSTRING("[%s] - Without Handshake has been set to %s"), GetServerName(), bWithoutHandshake ? "TRUE" : "FALSE");
	}
}

void Server::SetCalcLatencyInterval(const long sCalcLatencyInterval)
{
	this->sCalcLatencyInterval = sCalcLatencyInterval;

	if (strcmp(GetServerName(), DEFAULT_WEBSERVER_SERVERNAME) == 0)
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

float Server::GetTimeSpamProtection() const
{
	return sTimeSpamProtection;
}

long long Server::GetFrequenz() const
{
	return sfrequenz;
}

unsigned int Server::GetMaxPeers() const
{
	return sMaxPeers;
}

bool Server::GetSSL() const
{
	return sSSL;
}

const char* Server::GetCertFileName() const
{
	return sCertFileName;
}

const char* Server::GetKeyFileName() const
{
	return sKeyFileName;
}

const char* Server::GetCaFileName() const
{
	return sCaFileName;
}

bool Server::GetCustomHandshakeMethode() const
{
	return hUseCustom;
}

CPOINTER<char> Server::GetHandshakeOriginCompare() const
{
	return hOrigin;
}

bool Server::GetCompressPackage() const
{
	return sCompressPackage;
}

long Server::GetTCPReadTimeout() const
{
	return sTCPReadTimeout;
}

bool Server::GetWithoutHandshake() const
{
	return bWithoutHandshake;
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

void Server::network_t::setDataFragmented(byte* pointer)
{
	deallocDataFragmented();
	_dataFragment = pointer;
}

void Server::network_t::allocData(const size_t size)
{
	clear();
	_data = ALLOC<byte>(size + 1);
	memset(getData(), NULL, size * sizeof(byte));
	getData()[size] = '\0';

	setDataSize(size);
}

void Server::network_t::allocDataFragmented(const size_t size)
{
	clear();
	_dataFragment = ALLOC<byte>(size + 1);
	memset(getDataFragmented(), NULL, size * sizeof(byte));
	getDataFragmented()[size] = '\0';

	setDataSize(size);
}

void Server::network_t::deallocData()
{
	_data.free();
}

void Server::network_t::deallocDataFragmented()
{
	_dataFragment.free();

}
byte* Server::network_t::getData() const
{
	return _data.get();
}

byte* Server::network_t::getDataFragmented() const
{
	return _dataFragment.get();
}

void Server::network_t::reset()
{
	memset(_dataReceive, NULL, DEFAULT_WEBSERVER_MAX_PACKET_SIZE * sizeof(byte));
}

void Server::network_t::clear()
{
	deallocData();
	deallocDataFragmented();
	_data_size = NULL;
}

void Server::network_t::setDataSize(const size_t size)
{
	_data_size = size;
}

size_t Server::network_t::getDataSize() const
{
	return _data_size;
}

void Server::network_t::setDataFragmentSize(const size_t size)
{
	_data_sizeFragment = size;
}

size_t Server::network_t::getDataFragmentSize() const
{
	return _data_sizeFragment;
}

bool Server::network_t::dataValid() const
{
	return _data.valid();
}

bool Server::network_t::dataFragmentValid() const
{
	return _dataFragment.valid();
}

byte* Server::network_t::getDataReceive()
{
	return _dataReceive;
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

Server::NET_PEER Server::CreatePeer(const sockaddr_in client_addr, const SOCKET socket)
{
	// UniqueID is equal to socket, since socket is already an unique ID
	const auto peer = new NET_IPEER();
	peer->UniqueID = socket;
	peer->pSocket = socket;
	peer->client_addr = client_addr;

	/* Set Read Timeout */
	timeval tv = {};
	tv.tv_sec = GetTCPReadTimeout();
	tv.tv_usec = 0;
	setsockopt(peer->pSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	IncreasePeersCounter();

	// callback
	OnPeerConnect(peer);

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): connected!"), GetServerName(), peer->IPAddr().get());
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

		// callback
		OnPeerDisconnect(peer);

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): disconnected!"), GetServerName(), peer->IPAddr().get());

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

	peer->unlock();
	return false;
}

void Server::UpdatePeer(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	// Calculate latency interval
	if (peer->lastCalcLatency < CURRENTCLOCKTIME)
	{
		std::thread(LatencyThread, peer).detach();
		peer->lastCalcLatency = CREATETIMER(GetCalcLatencyInterval());
	}
}

void Server::NET_IPEER::clear()
{
	UniqueID = INVALID_UID;
	pSocket = INVALID_SOCKET;
	client_addr = sockaddr_in();
	estabilished = false;
	isAsync = false;
	handshake = false;
	bLatency = false;
	latency = -1;
	lastCalcLatency = 0;

	network.clear();
	network.reset();

	if (ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = nullptr;
	}
}

void Server::NET_IPEER::setAsync(const bool status)
{
	isAsync = status;
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

void Server::DisconnectPeer(NET_PEER peer, const int code)
{
	PEER_NOT_VALID(peer,
		return;
	);

	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): has been disconnected, reason: %s"), GetServerName(), peer->IPAddr().get(), Net::Codes::NetGetErrorMessage(code));

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

bool Server::Start(const char* serverName, const u_short serverPort, const ssl::NET_SSL_METHOD Method)
{
	if (IsRunning())
		return false;

	/* set name and port */
	SetServerName(serverName);
	SetServerPort(serverPort);

	/* SSL */
	if (GetSSL())
	{
		/* Init SSL */
		SSL_load_error_strings();
		OpenSSL_add_ssl_algorithms();

		// create CTX
		ctx = SSL_CTX_new(NET_CREATE_SSL_OBJECT(Method));
		if (!ctx)
		{
			LOG_ERROR(CSTRING("[%s] - ctx is NULL"), GetServerName());
			return false;
		}

		/* Set the key and cert */
		if (SSL_CTX_use_certificate_file(ctx, GetCertFileName(), SSL_FILETYPE_PEM) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), GetServerName(), GetCertFileName());
			return false;
		}

		if (SSL_CTX_use_PrivateKey_file(ctx, GetKeyFileName(), SSL_FILETYPE_PEM) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), GetServerName(), GetKeyFileName());
			return false;
		}

		/* load verfiy location (CA)*/
		if (SSL_CTX_load_verify_locations(ctx, GetCaFileName(), nullptr) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), GetServerName(), GetCaFileName());
			return false;
		}

		/* verify private key */
		if (!SSL_CTX_check_private_key(ctx))
		{
			LOG_ERROR(CSTRING("[%s] - Private key does not match with the public certificate"), GetServerName());
			ERR_print_errors_fp(stderr);
			return false;
		}

		SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
		/*	SSL_CTX_set_info_callback(ctx, [](const SSL* ssl, int type, int value)
			{
					LOG(CSTRING("CALLBACK CTX SET INFO!"));
			});*/

		LOG_DEBUG(CSTRING("[%s] - Server is using method: %s"), GetServerName(), Net::ssl::GET_SSL_METHOD_NAME(Method).data());
	}

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

	if (res != 0)
	{
		LOG_ERROR(CSTRING("[%s] - getaddrinfo failed with error: %d"), GetServerName(), res);
		WSACleanup();
		return false;
	}

	// Create a SOCKET for connecting to server
	SetListenSocket(socket(result->ai_family, result->ai_socktype, result->ai_protocol));

	if (GetListenSocket() == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[%s] - socket failed with error: %ld"), GetServerName(), WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Set the mode of the socket to be nonblocking
	u_long iMode = 1;
	res = ioctlsocket(GetListenSocket(), FIONBIO, &iMode);

	if (res == SOCKET_ERROR) {
		LOG_ERROR(CSTRING("[%s] - ioctlsocket failed with error: %d"), GetServerName(), WSAGetLastError());
		closesocket(GetListenSocket());
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	res = bind(GetListenSocket(), result->ai_addr, (int)result->ai_addrlen);

	if (res == SOCKET_ERROR)
	{
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

short Server::Handshake(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return WebServerHandshake::HandshakeRet_t::peer_not_valid;
	);

	/* SSL */
	if (peer->ssl)
	{
		// check socket still open
		if (recv(peer->pSocket, nullptr, NULL, 0) == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSAETIMEDOUT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): timouted!"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAECONNRESET:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): has been forced to disconnect!"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			default:
				break;
			}
		}

		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), DEFAULT_WEBSERVER_MAX_PACKET_SIZE);
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			switch (err)
			{
			case SSL_ERROR_ZERO_RETURN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_WANT_CONNECT:
			case SSL_ERROR_WANT_ACCEPT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_WANT_X509_LOOKUP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_SYSCALL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_SSL:
				/* Some servers did not close the connection properly */
				peer->network.reset();
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_WANT_READ:
				peer->network.reset();
				return WebServerHandshake::HandshakeRet_t::would_block;

			default:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;
			}
		}
		ERR_clear_error();
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
	}
	else
	{
		const auto data_size = recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), DEFAULT_WEBSERVER_MAX_PACKET_SIZE, 0);
		if (data_size == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAENETDOWN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEFAULT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAENOTCONN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEINTR:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEINPROGRESS:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAENETRESET:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAENOTSOCK:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEOPNOTSUPP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAESHUTDOWN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEWOULDBLOCK:
				peer->network.reset();
				return WebServerHandshake::HandshakeRet_t::would_block;

			case WSAEMSGSIZE:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAEINVAL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAECONNABORTED:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAETIMEDOUT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case WSAECONNRESET:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			default:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;
			}
		}
		if (data_size == 0)
		{
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), GetServerName(), peer->IPAddr().get());
			ErasePeer(peer);
			return WebServerHandshake::HandshakeRet_t::error;
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
	}

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

		NET_SHA1 sha;
		unsigned int message_digest[5];

		const auto stringSecWebSocketKey = CSTRING("Sec-WebSocket-Key");

		sha.Reset();
		sha << entries[stringSecWebSocketKey].data();
		sha << CSTRING("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		sha.Result(message_digest);
		// convert sha1 hash bytes to network byte order because this sha1
		//  library works on ints rather than bytes
		for (auto& entry : message_digest)
			entry = htonl(entry);

		// Encode Base64
		size_t outlen = 0;
		byte* enc_Sec_Key = ALLOC<BYTE>(20 + 1);
		memcpy(enc_Sec_Key, message_digest, 20);
		enc_Sec_Key[20] = '\0';
		NET_BASE64::encode(enc_Sec_Key, outlen);

		char host[15];
		if (GetCustomHandshakeMethode())
			sprintf_s(host, CSTRING("%s:%i"), GetHandshakeOriginCompare().get(), GetServerPort());
		else
			sprintf_s(host, CSTRING("127.0.0.1:%i"), GetServerPort());

		CPOINTER<char> origin;
		if (GetCustomHandshakeMethode())
		{
			const auto originSize = strlen(GetHandshakeOriginCompare().get());
			origin = ALLOC<char>(originSize + 1);
			sprintf(origin.get(), CSTRING("%s%s"), GetSSL() ? CSTRING("https://") : CSTRING("http://"), GetHandshakeOriginCompare().get());
		}

		// Create Response
		CPOINTER<char> buffer(ALLOC<char>(DEFAULT_WEBSERVER_MAX_PACKET_SIZE + 1));
		sprintf(buffer.get(), CSTRING("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n"), reinterpret_cast<char*>(enc_Sec_Key));

		/* SSL */
		if (peer->ssl)
		{
			auto res = -1;
			do
			{
				if (!peer)
					return false;

				SOCKET_NOT_VALID(peer->pSocket)
					return false;

				if (send(peer->pSocket, nullptr, NULL, 0) == SOCKET_ERROR)
				{
					ErasePeer(peer);
					LOG_ERROR(CSTRING("[%s] - Failed to send Package, reason: Socket Error"), GetServerName());
					return WebServerHandshake::HandshakeRet_t::error;
				}

				res = SSL_write(peer->ssl, buffer.get(), static_cast<int>(strlen(buffer.get())));
				if (res <= 0)
				{
					switch (SSL_get_error(peer->ssl, res))
					{
					case SSL_ERROR_ZERO_RETURN:
						onSSLTimeout(peer);
						return WebServerHandshake::HandshakeRet_t::error;

					case SSL_ERROR_SYSCALL:
						onSSLTimeout(peer);
						return WebServerHandshake::HandshakeRet_t::error;

					default:
						break;
					}
				}
				ERR_clear_error();
			} while (res <= 0);
		}
		else
		{
			auto size = static_cast<int>(strlen(buffer.get()));
			auto res = 0;
			do
			{
				res = send(peer->pSocket, buffer.get(), size, 0);
				if (res == SOCKET_ERROR)
				{
					ErasePeer(peer);
					LOG_ERROR(CSTRING("[%s] - Failed to send Package, reason: Socket Error"), GetServerName());
					return WebServerHandshake::HandshakeRet_t::error;
				}

				size -= res;
			} while (size > 0);
		}

		buffer.free();

		// clear data
		peer->network.clear();

		const auto stringUpdate = CSTRING("Upgrade");
		const auto stringHost = CSTRING("Host");
		const auto stringOrigin = CSTRING("Origin");

		/* Handshake Failed - Display Error Message */
		if (GetCustomHandshakeMethode())
		{
			if (!(strcmp(entries[stringUpdate].data(), CSTRING("websocket")) == 0 && strcmp(reinterpret_cast<char*>(enc_Sec_Key), CSTRING("")) != 0 && strcmp(entries[stringHost].data(), host) == 0 && strcmp(entries[stringOrigin].data(), origin.get()) == 0))
			{
				origin.free();
				LOG_ERROR(CSTRING("[%s] - Handshake failed:\nReceived from Peer ('%s'):\nUpgrade: %s\nHost: %s\nOrigin: %s"), GetServerName(), peer->IPAddr().get(), entries[stringUpdate].data(), entries[stringHost].data(), entries[stringOrigin].data());
				return WebServerHandshake::HandshakeRet_t::missmatch;
			}
		}
		else
		{
			if (!(strcmp(entries[stringUpdate].data(), CSTRING("websocket")) == 0 && strcmp(reinterpret_cast<char*>(enc_Sec_Key), CSTRING("")) != 0))
			{
				origin.free();
				LOG_ERROR(CSTRING("[%s] - Handshake failed:\nReceived from Peer ('%s'):\nUpgrade: %s"), GetServerName(), peer->IPAddr().get(), entries[stringUpdate].data());
				return WebServerHandshake::HandshakeRet_t::missmatch;
			}
		}

		origin.free();
		FREE(enc_Sec_Key);
		return WebServerHandshake::HandshakeRet_t::success;
	}

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen on Handshake"), GetServerName(), peer->IPAddr().get());

	// clear data
	peer->network.clear();
	return WebServerHandshake::HandshakeRet_t::error;
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
				LOG_ERROR(CSTRING("[%s] - Failure on settings socket option { 0x%ld : %i }"), GetServerName(), entry.opt, GetLastError());
		}

		std::thread(&Server::ReceiveThread, this, client_addr, GetAcceptSocket()).detach();
	}
}

bool Server::NeedExit() const
{
	return DoExit;
}

void Server::DoSend(NET_PEER peer, const int id, NET_PACKAGE pkg, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

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

	EncodeFrame(buffer.GetString(), buffer.GetLength(), peer, opc);
}

void Server::EncodeFrame(const char* in_frame, const size_t frame_length, NET_PEER peer, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	/* ENCODE FRAME */
	auto frameCount = static_cast<int>(ceil((float)frame_length / DEFAULT_WEBSERVER_MAX_PACKET_SIZE));
	if (frameCount == 0)
		frameCount = 1;

	const auto maxFrame = frameCount - 1;
	const int lastFrameBufferLength = (frame_length % DEFAULT_WEBSERVER_MAX_PACKET_SIZE) != 0 ? (frame_length % DEFAULT_WEBSERVER_MAX_PACKET_SIZE) : (frame_length != 0 ? DEFAULT_WEBSERVER_MAX_PACKET_SIZE : 0);

	for (auto i = 0; i < frameCount; i++)
	{
		const unsigned char fin = i != maxFrame ? 0 : WS_FIN;
		const unsigned char opcode = i != 0 ? OPCODE_CONTINUE : opc;

		const size_t bufferLength = i != maxFrame ? DEFAULT_WEBSERVER_MAX_PACKET_SIZE : lastFrameBufferLength;
		CPOINTER<char> buf;
		size_t totalLength;

		if (bufferLength <= 125)
		{
			totalLength = bufferLength + 2;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = (char)bufferLength;
			memcpy(buf.get() + 2, in_frame, frame_length);
		}
		else if (bufferLength <= 65535)
		{
			totalLength = bufferLength + 4;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = WS_PAYLOAD_LENGTH_16;
			buf.get()[2] = (char)bufferLength >> 8;
			buf.get()[3] = (char)bufferLength;
			memcpy(buf.get() + 4, in_frame, frame_length);
		}
		else
		{
			totalLength = bufferLength + 10;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = WS_PAYLOAD_LENGTH_63;
			buf.get()[2] = 0;
			buf.get()[3] = 0;
			buf.get()[4] = 0;
			buf.get()[5] = 0;
			buf.get()[6] = (char)bufferLength >> 24;
			buf.get()[7] = (char)bufferLength >> 16;
			buf.get()[8] = (char)bufferLength >> 8;
			buf.get()[9] = (char)bufferLength;
			memcpy(buf.get() + 10, in_frame, frame_length);
		}

		if (!buf.valid())
		{
			LOG_DEBUG(CSTRING("[%s] - Buffer is nullptr!"), GetServerName());
			return;
		}

		/* SSL */
		if (peer->ssl)
		{
			int res;
			do
			{
				if (!peer)
				{
					buf.free();
					return;
				}

				SOCKET_NOT_VALID(peer->pSocket)
					return;

				res = SSL_write(peer->ssl, reinterpret_cast<char*>(buf.get()), static_cast<int>(totalLength));
				if (res <= 0)
				{
					const auto err = SSL_get_error(peer->ssl, res);
					switch (err)
					{
					case SSL_ERROR_ZERO_RETURN:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_WANT_CONNECT:
					case SSL_ERROR_WANT_ACCEPT:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_WANT_X509_LOOKUP:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_SYSCALL:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_SSL:
						buf.free();
						return;

					case SSL_ERROR_WANT_WRITE:
						continue;

					default:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;
					}
				}
				ERR_clear_error();
			} while (res <= 0);
		}
		else
		{
			auto sendSize = totalLength;
			do
			{
				const auto res = send(peer->pSocket, reinterpret_cast<char*>(buf.get()), static_cast<int>(totalLength), 0);
				if (res == SOCKET_ERROR)
				{
					switch (WSAGetLastError())
					{
					case WSANOTINITIALISED:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAENETDOWN:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEACCES:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEINTR:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEINPROGRESS:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEFAULT:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAENETRESET:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAENOBUFS:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): No buffer space is available"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAENOTCONN:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAENOTSOCK:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEOPNOTSUPP:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAESHUTDOWN:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEWOULDBLOCK:
						continue;

					case WSAEMSGSIZE:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEHOSTUNREACH:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The remote host cannot be reached from this host at this time"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAEINVAL:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAECONNABORTED:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAECONNRESET:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case WSAETIMEDOUT:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped, because of a network failure or because the system on the other end went down without notice"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					default:
						buf.free();
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), GetServerName(), peer->IPAddr().get());
						ErasePeer(peer);
						return;
					}
				}

				sendSize -= res;
			} while (sendSize > 0);
		}

		buf.free();
	}
	///////////////////////
}

void Server::ReceiveThread(const sockaddr_in client_addr, const SOCKET socket)
{
	auto peer = CreatePeer(client_addr, socket);

	/* Handshake */
	if (!GetWithoutHandshake())
	{
		do
		{
			peer->setAsync(true);
			const auto res = Handshake(peer);
			if (res == WebServerHandshake::peer_not_valid)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to invalid socket!"), GetServerName(), peer->IPAddr().get());

				// erase him
				peer->setAsync(false);
				ErasePeer(peer);
				return;
			}
			if (res == WebServerHandshake::would_block)
			{
				peer->setAsync(false);
				continue;
			}
			if (res == WebServerHandshake::missmatch)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake missmatch!"), GetServerName(), peer->IPAddr().get());

				// erase him
				peer->setAsync(false);
				ErasePeer(peer);
				return;
			}
			if (res == WebServerHandshake::error)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake error!"), GetServerName(), peer->IPAddr().get());

				peer->setAsync(false);
				ErasePeer(peer);
				return;
			}
			if (res == WebServerHandshake::success)
			{
				peer->setAsync(false);
				peer->handshake = true;
				break;
			}
			peer->setAsync(false);
		} while (!DoExit);

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): has been successfully handshaked"), GetServerName(), peer->IPAddr().get());
	}
	peer->estabilished = true;
	OnPeerEstabilished(peer);

	while (peer)
	{
		if (!IsRunning() || NeedExit())
			break;

		UpdatePeer(peer);
		OnPeerUpdate(peer);

		SOCKET_VALID(peer->pSocket)
		{
			peer->setAsync(true);
			DoReceive(peer);
			peer->setAsync(false);
		}
		else
		{
			peer->setAsync(false);
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	}

	// wait until thread has finished
	while (peer && peer->bLatency)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	};

	// erase him
	peer->setAsync(false);
	ErasePeer(peer);

	delete peer;
	peer = nullptr;
}

void Server::DoReceive(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (peer->ssl)
	{
		// check socket still open
		if (recv(peer->pSocket, nullptr, NULL, 0) == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEMSGSIZE:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				break;

			default:
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}

		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), DEFAULT_WEBSERVER_MAX_PACKET_SIZE);
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			switch (err)
			{
			case SSL_ERROR_ZERO_RETURN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case SSL_ERROR_WANT_CONNECT:
			case SSL_ERROR_WANT_ACCEPT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case SSL_ERROR_WANT_X509_LOOKUP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case SSL_ERROR_SYSCALL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case SSL_ERROR_SSL:
				/* Some servers did not close the connection properly */
				peer->network.reset();
				return;

			case SSL_ERROR_WANT_READ:
				peer->network.reset();
				return;

			default:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}
		ERR_clear_error();
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
	}
	else
	{
		const auto data_size = recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), DEFAULT_WEBSERVER_MAX_PACKET_SIZE, 0);
		if (data_size == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A successful WSAStartup() call must occur before using this function"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETDOWN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The network subsystem has failed"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEFAULT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The buf parameter is not completely contained in a valid part of the user address space"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTCONN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket is not connected"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINTR:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The (blocking) call was canceled through WSACancelBlockingCall()"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINPROGRESS:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENETRESET:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAENOTSOCK:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The descriptor is not a socket"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEOPNOTSUPP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAESHUTDOWN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEWOULDBLOCK:
				peer->network.reset();
				return;

			case WSAEMSGSIZE:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The message was too large to fit into the specified buffer and was truncated"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAEINVAL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNABORTED:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAETIMEDOUT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The connection has been dropped because of a network failure or because the peer system failed to respond"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			case WSAECONNRESET:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;

			default:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), GetServerName(), peer->IPAddr().get());
				ErasePeer(peer);
				return;
			}
		}
		if (data_size == 0)
		{
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), GetServerName(), peer->IPAddr().get());
			ErasePeer(peer);
			return;
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
	}

	DecodeFrame(peer);
}

void Server::DecodeFrame(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	// 6 bytes have to be set (2 bytes to read the mask and 4 bytes represent the mask key)
	if (peer->network.getDataSize() < 6)
	{
		peer->network.clear();
		return;
	}

	/* DECODE FRAME */
	const unsigned char FIN = peer->network.getData()[0] & WS_FIN;
	const unsigned char OPC = peer->network.getData()[0] & WS_OPCODE;
	if (OPC == OPCODE_CLOSE)
	{
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), GetServerName(), peer->IPAddr().get());
		ErasePeer(peer);
		return;
	}
	if (OPC == OPCODE_PING)
	{
		Package pong;
		DoSend(peer, WS_CONTROL_PACKAGE, pong, OPCODE_PONG);
		peer->network.clear();
		return;
	}
	if (OPC == OPCODE_PONG)
	{
		// todo work with timer
		peer->network.clear();
		return;
	}

	const auto IsMasked = peer->network.getData()[1] > WS_MASK;
	unsigned int PayloadLength = peer->network.getData()[1] & WS_PAYLOADLENGTH;
	auto NextBits = (16 / 8); // read the next 16 bits
	if (PayloadLength == 126)
	{
		const auto OffsetLen126 = (16 / 8);
		byte tmpRead[OffsetLen126] = {};
		memcpy(tmpRead, &peer->network.getData()[NextBits], OffsetLen126);
		PayloadLength = (unsigned int)ntohs(*(u_short*)tmpRead);
		NextBits += OffsetLen126;
	}
	else if (PayloadLength == 127)
	{
		const auto OffsetLen127 = (64 / 8);
		byte tmpRead[OffsetLen127] = {};
		memcpy(tmpRead, &peer->network.getData()[NextBits], OffsetLen127);
		PayloadLength = (unsigned int)ntohll(*(unsigned long long*)tmpRead);
		NextBits += OffsetLen127;

		//// MSB (The most significant bit MUST be 0)
		//const auto msb = 1 << 63;
		//if (PayloadLength & msb)
		//{
		//	peer->clearData();
		//	return;
		//}
	}

	if (IsMasked)
	{
		// Read the Mask Key
		const auto MaskKeyLength = (32 / 8);
		byte MaskKey[MaskKeyLength + 1] = {};
		memcpy(MaskKey, &peer->network.getData()[NextBits], MaskKeyLength);
		MaskKey[MaskKeyLength] = '\0';

		NextBits += MaskKeyLength;

		// Decode Payload
		CPOINTER<byte> Payload(ALLOC<byte>(PayloadLength + 1));
		memcpy(Payload.get(), &peer->network.getData()[NextBits], PayloadLength);
		Payload.get()[PayloadLength] = '\0';

		for (unsigned int i = 0; i < PayloadLength; ++i)
			Payload.get()[i] = (Payload.get()[i] ^ MaskKey[i % 4]);

		peer->network.clear();
		peer->network.allocData(PayloadLength);
		memcpy(peer->network.getData(), Payload.get(), PayloadLength);
		peer->network.getData()[PayloadLength] = '\0';
		peer->network.setDataSize(PayloadLength);
		Payload.free();
	}
	else
	{
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been closed due to unmasked message"), GetServerName(), peer->IPAddr().get());
		ErasePeer(peer);
		return;
	}

	// frame is not complete
	if (!FIN)
	{
		if (OPC == WS_CONTROLFRAME)
		{
			peer->network.clear();
			return;
		}

		if (!memcmp(peer->network.getData(), CSTRING(""), strlen(reinterpret_cast<char const*>(peer->network.getData()))))
		{
			peer->network.clear();
			return;
		}

		if (!peer->network.dataFragmentValid())
		{
			peer->network.setDataFragmented(ALLOC<byte>(peer->network.getDataSize() + 1));
			memcpy(peer->network.getDataFragmented(), peer->network.getData(), peer->network.getDataSize());
			peer->network.getDataFragmented()[peer->network.getDataSize()] = '\0';
			peer->network.setDataFragmentSize(peer->network.getDataSize());
		}
		else
		{
			// append
			const auto dataFragementSize = strlen(reinterpret_cast<char const*>(peer->network.getDataFragmented()));
			CPOINTER<byte> tmpDataFragement(ALLOC<byte>(dataFragementSize + 1));
			memcpy(tmpDataFragement.get(), peer->network.getDataFragmented(), dataFragementSize);
			memcpy(&tmpDataFragement.get()[dataFragementSize], peer->network.getData(), peer->network.getDataSize());
			tmpDataFragement.get()[(peer->network.getDataSize() + dataFragementSize)] = '\0';
			peer->network.setDataFragmented(ALLOC<byte>((peer->network.getDataSize() + dataFragementSize) + 1));
			memcpy(peer->network.getDataFragmented(), tmpDataFragement.get(), (peer->network.getDataSize() + dataFragementSize));
			peer->network.getDataFragmented()[(peer->network.getDataSize() + dataFragementSize)] = '\0';
			peer->network.setDataFragmentSize(peer->network.getDataFragmentSize() + peer->network.getDataSize());
			tmpDataFragement.free();
		}
	}
	else
	{
		if (!memcmp(peer->network.getData(), CSTRING(""), strlen(reinterpret_cast<char const*>(peer->network.getData()))))
		{
			peer->network.clear();
			return;
		}

		// this frame is complete
		if (OPC != OPCODE_CONTINUE)
		{
			// this is a message frame process the message
			if (OPC == OPCODE_TEXT || OPC == OPCODE_BINARY)
			{
				if (!peer->network.dataFragmentValid())
				{
					//LOG(CSTRING("Executing complete data frame: %llu\nPayload: %s"), peer->data_size, reinterpret_cast<const char*>(peer->data));
					ProcessPackage(peer, peer->network.getData(), peer->network.getDataSize());
				}
				else
				{
					//LOG(CSTRING("Executing data fragement: %llu\nPayload fragment: %s"), peer->data_sizeFragment, reinterpret_cast<const char*>(peer->data_sizeFragment));
					ProcessPackage(peer, peer->network.getDataFragmented(), peer->network.getDataFragmentSize());
					peer->network.setDataFragmented(nullptr);
					peer->network.setDataFragmentSize(NULL);
				}
			}

			peer->network.clear();
			return;
		}

		peer->network.setDataFragmented(ALLOC<byte>(peer->network.getDataSize() + 1));
		memcpy(peer->network.getDataFragmented(), peer->network.getData(), peer->network.getDataSize());
		peer->network.getDataFragmented()[peer->network.getDataSize()] = '\0';
		peer->network.setDataFragmentSize(peer->network.getDataSize());
	}

	peer->network.clear();
}

bool Server::ProcessPackage(NET_PEER peer, BYTE* data, const size_t size)
{
	PEER_NOT_VALID(peer,
		return false;
	);
	
	if (!data)
		return false;

	Package PKG;
	PKG.Parse(reinterpret_cast<char*>(data));
	if (!PKG.GetPackage().HasMember(CSTRING("ID")))
	{
		LOG_PEER(CSTRING("Missing member 'ID' in the package"));
		return false;
	}

	const auto id = PKG.GetPackage().FindMember(CSTRING("ID"))->value.GetInt();
	if (id < 0)
	{
		LOG_PEER(CSTRING("Member 'ID' is smaller than zero and gets blocked"));
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedPackage);
		return false;
	}

	if (!PKG.GetPackage().HasMember(CSTRING("CONTENT")))
	{
		LOG_PEER(CSTRING("Missing member 'CONTENT' in the package"));
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedPackage);
		return false;
	}

	Package Content;

	if (!PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.IsNull())
		Content.SetPackage(PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.GetObject());

	if (!CheckDataN(peer, id, Content))
	{
		if (!CheckData(peer, id, Content))
		{
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedPackage);
			return false;
		}
	}

	return true;
}

void Server::onSSLTimeout(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);
	
	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): timouted!"), GetServerName(), peer->IPAddr().get());
	ErasePeer(peer);
}

size_t Server::getCountPeers() const
{
	return _CounterPeersTable;
}

NET_SERVER_BEGIN_DATA_PACKAGE_NATIVE(Server)
NET_SERVER_END_DATA_PACKAGE
NET_NAMESPACE_END
NET_NAMESPACE_END