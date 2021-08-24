#include "WebServer.h"
#include <Net/Import/Kernel32.h>
#include <Net/Import/Ws2_32.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(WebServer)
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
	optionBitFlag = NULL;
	socketOptionBitFlag = NULL;
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
	memset(_dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE * sizeof(byte));
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

NET_THREAD(LatencyTick)
{
	const auto peer = (Server::NET_PEER)parameter;
	if (!peer) return NULL;

	LOG_DEBUG(CSTRING("[NET] - LatencyTick thread has been started"));
	// tmp disabled
	//peer->latency = Net::Protocol::ICMP::Exec(peer->IPAddr().get());
	LOG_DEBUG(CSTRING("[NET] - LatencyTick thread has been end"));
	return NULL;
}

struct DoCalcLatency_t
{
	Server* server;
	Server::NET_PEER peer;
};

NET_TIMER(DoCalcLatency)
{
	const auto info = (DoCalcLatency_t*)param;
	if (!info) NET_STOP_TIMER;

	const auto server = info->server;
	const auto peer = info->peer;

	Thread::Create(LatencyTick, peer);
	Timer::SetTime(peer->hCalcLatency, server->Isset(NET_OPT_INTERVAL_LATENCY) ? server->GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY);
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
	Ws2_32::setsockopt(peer->pSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	if (Isset(NET_OPT_SSL) ? GetOption<bool>(NET_OPT_SSL) : NET_OPT_DEFAULT_SSL)
	{
		peer->ssl = SSL_new(ctx);
		SSL_set_accept_state(peer->ssl); /* sets ssl to work in server mode. */

		/* Attach SSL to the socket */
		SSL_set_fd(peer->ssl, static_cast<int>(GetAcceptSocket()));

		/* Establish TLS connection */
		const auto res = SSL_accept(peer->ssl);
		if (res == 0)
		{
			LOG_ERROR("[%s] - The TLS/SSL handshake was not successful but was shut down controlled and by the specifications of the TLS/SSL protocol", SERVERNAME(this));
			return nullptr;
		}
		if (res < 0)
		{
			const auto err = SSL_get_error(peer->ssl, res);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				LOG_DEBUG("[%s] - The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed", SERVERNAME(this));
				return nullptr;
			}
			if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
			{
				LOG_DEBUG("[%s] - The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used", SERVERNAME(this));
				return nullptr;
			}
			if (err == SSL_ERROR_WANT_X509_LOOKUP)
			{
				LOG_DEBUG("[%s] - The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application", SERVERNAME(this));
				return nullptr;
			}
			if (err == SSL_ERROR_SYSCALL)
			{
				LOG_DEBUG("[%s] - Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details", SERVERNAME(this));
				return nullptr;
			}
			if (err == SSL_ERROR_SSL)
			{
				LOG_DEBUG("[%s] - A non-recoverable, fatal error in the SSL library occurred, usually a protocol error. The OpenSSL error queue contains more information on the error. If this error occurs then no further I/O operations should be performed on the connection and SSL_shutdown() must not be called", SERVERNAME(this));
				return nullptr;
			}
			ERR_clear_error();
		}
	}
	else
		peer->ssl = nullptr;

	if (Isset(NET_OPT_DISABLE_LATENCY_REQUEST) ? GetOption<bool>(NET_OPT_DISABLE_LATENCY_REQUEST) : NET_OPT_DEFAULT_LATENCY_REQUEST)
	{
		const auto _DoCalcLatency = new DoCalcLatency_t();
		_DoCalcLatency->server = this;
		_DoCalcLatency->peer = peer;
		peer->hCalcLatency = Timer::Create(DoCalcLatency, Isset(NET_OPT_INTERVAL_LATENCY) ? GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY, _DoCalcLatency, true);
	}

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

	std::lock_guard<std::recursive_mutex> guard(peer->critical);

	if (clear)
	{
		if (peer->hCalcLatency)
		{
			// stop latency interval
			Timer::WaitSingleObjectStopped(peer->hCalcLatency);
			peer->hCalcLatency = nullptr;
		}

		// callback
		OnPeerDisconnect(peer);

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

void Server::NET_IPEER::clear()
{
	UniqueID = INVALID_UID;
	pSocket = INVALID_SOCKET;
	client_addr = sockaddr_in();
	estabilished = false;
	bErase = false;
	handshake = false;
	latency = -1;
	hCalcLatency = nullptr;

	network.clear();
	network.reset();

	if (ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = nullptr;
	}
}

IPRef Server::NET_IPEER::IPAddr() const
{
	const auto buf = ALLOC<char>(INET_ADDRSTRLEN);
#ifdef VS13
	return IPRef(Ws2_32::inet_ntop(AF_INET, (PVOID)&client_addr.sin_addr, buf, INET_ADDRSTRLEN));
#else
	return IPRef(Ws2_32::inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN));
#endif
}

void Server::DisconnectPeer(NET_PEER peer, const int code)
{
	PEER_NOT_VALID(peer,
		return;
	);

	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): has been disconnected, reason: %s"), SERVERNAME(this), peer->IPAddr().get(), Net::Codes::NetGetErrorMessage(code));

	// now after we have sent him the reason, close connection
	ErasePeer(peer);
}

/* Thread functions */
NET_THREAD(TickThread)
{
	const auto server = (Server*)parameter;
	if (!server) return NULL;

	LOG_DEBUG(CSTRING("[NET] - Tick thread has been started"));
	while (server->IsRunning())
	{
		server->Tick();
#ifdef BUILD_LINUX
		usleep(FREQUENZ(server));
#else
		Kernel32::Sleep(FREQUENZ(server));
#endif
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
#ifdef BUILD_LINUX
		usleep(FREQUENZ(server));
#else
		Kernel32::Sleep(FREQUENZ(server));
#endif
	}
	LOG_DEBUG(CSTRING("[NET] - Acceptor thread has been end"));
	return NULL;
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

bool Server::Run()
{
	if (IsRunning())
		return false;

	/* SSL */
	if (Isset(NET_OPT_SSL) ? GetOption<bool>(NET_OPT_SSL) : NET_OPT_DEFAULT_SSL)
	{
		/* Init SSL */
		SSL_load_error_strings();
		OpenSSL_add_ssl_algorithms();

		// create CTX
		ctx = SSL_CTX_new(Net::ssl::NET_CREATE_SSL_OBJECT(Isset(NET_OPT_SSL_METHOD) ? GetOption<int>(NET_OPT_SSL_METHOD) : NET_OPT_DEFAULT_SSL_METHOD));
		if (!ctx)
		{
			LOG_ERROR(CSTRING("[%s] - ctx is NULL"), SERVERNAME(this));
			return false;
		}

		/* Set the key and cert */
		if (SSL_CTX_use_certificate_file(ctx, Isset(NET_OPT_SSL_CERT) ? GetOption<char*>(NET_OPT_SSL_CERT) : NET_OPT_DEFAULT_SSL_CERT, SSL_FILETYPE_PEM) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_CERT) ? GetOption<char*>(NET_OPT_SSL_CERT) : NET_OPT_DEFAULT_SSL_CERT);
			return false;
		}

		if (SSL_CTX_use_PrivateKey_file(ctx, Isset(NET_OPT_SSL_KEY) ? GetOption<char*>(NET_OPT_SSL_KEY) : NET_OPT_DEFAULT_SSL_KEY, SSL_FILETYPE_PEM) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_KEY) ? GetOption<char*>(NET_OPT_SSL_KEY) : NET_OPT_DEFAULT_SSL_KEY);
			return false;
		}

		/* load verfiy location (CA)*/
		NET_FILEMANAGER fmanager(Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
		if (!fmanager.file_exists())
		{
			LOG_ERROR(CSTRING("[%s] - File does not exits '%s'"), SERVERNAME(this), Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
			return false;
		}

		if (SSL_CTX_load_verify_locations(ctx, Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA, nullptr) <= 0)
		{
			LOG_ERROR(CSTRING("[%s] - Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
			return false;
		}

		/* verify private key */
		if (!SSL_CTX_check_private_key(ctx))
		{
			LOG_ERROR(CSTRING("[%s] - Private key does not match with the public certificate"), SERVERNAME(this));
			ERR_print_errors_fp(stderr);
			return false;
		}

		SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
		/*	SSL_CTX_set_info_callback(ctx, [](const SSL* ssl, int type, int value)
			{
					LOG(CSTRING("CALLBACK CTX SET INFO!"));
			});*/

		LOG_DEBUG(CSTRING("[%s] - Server is using method: %s"), SERVERNAME(this), Net::ssl::GET_SSL_METHOD_NAME(Isset(NET_OPT_SSL_METHOD) ? GetOption<int>(NET_OPT_SSL_METHOD) : NET_OPT_DEFAULT_SSL_METHOD).data());
	}

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
	u_long iMode = 1;
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
		Ws2_32::closesocket(GetListenSocket());

	if (GetAcceptSocket())
		Ws2_32::closesocket(GetAcceptSocket());

#ifndef BUILD_LINUX
	Ws2_32::WSACleanup();
#endif

	LOG_DEBUG(CSTRING("[%s] - Closed!"), SERVERNAME(this));
	return true;
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
		if (Ws2_32::recv(peer->pSocket, nullptr, NULL, 0) == SOCKET_ERROR)
		{
			peer->network.reset();

#ifdef BUILD_LINUX
			if (errno != EWOULDBLOCK)
#else
			if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
			{
				ErasePeer(peer);

#ifdef BUILD_LINUX
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

				return WebServerHandshake::HandshakeRet_t::error;
			}
		}

		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE);
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			switch (err)
			{
			case SSL_ERROR_ZERO_RETURN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_WANT_CONNECT:
			case SSL_ERROR_WANT_ACCEPT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_WANT_X509_LOOKUP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return WebServerHandshake::HandshakeRet_t::error;

			case SSL_ERROR_SYSCALL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), SERVERNAME(this), peer->IPAddr().get());
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
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), SERVERNAME(this), peer->IPAddr().get());
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
		const auto data_size = Ws2_32::recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
		if (data_size == SOCKET_ERROR)
		{
			peer->network.reset();

#ifdef BUILD_LINUX
			if (errno != EWOULDBLOCK)
#else
			if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
			{
				ErasePeer(peer);

#ifdef BUILD_LINUX
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

				return WebServerHandshake::HandshakeRet_t::error;
			}

			return WebServerHandshake::HandshakeRet_t::would_block;
		}
		if (data_size == 0)
		{
			peer->network.reset();
			LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), SERVERNAME(this), peer->IPAddr().get());
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
			entry = Ws2_32::htonl(entry);

		// Encode Base64
		size_t outlen = 20;
		byte* enc_Sec_Key = ALLOC<BYTE>(20 + 1);
		memcpy(enc_Sec_Key, message_digest, 20);
		enc_Sec_Key[20] = '\0';
		NET_BASE64::encode(enc_Sec_Key, outlen);

		char host[15];
		if (Isset(NET_OPT_WS_CUSTOM_HANDSHAKE) ? GetOption<bool>(NET_OPT_WS_CUSTOM_HANDSHAKE) : NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE)
			sprintf(host, CSTRING("%s:%i"), Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN, Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);
		else
			sprintf(host, CSTRING("127.0.0.1:%i"), Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);

		CPOINTER<char> origin;
		if (Isset(NET_OPT_WS_CUSTOM_HANDSHAKE) ? GetOption<bool>(NET_OPT_WS_CUSTOM_HANDSHAKE) : NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE)
		{
			const auto originSize = strlen(Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN);
			origin = ALLOC<char>(originSize + 1);
			sprintf(origin.get(), CSTRING("%s%s"), (Isset(NET_OPT_SSL) ? GetOption<bool>(NET_OPT_SSL) : NET_OPT_DEFAULT_SSL) ? CSTRING("https://") : CSTRING("http://"), Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN);
		}

		// Create Response
		CPOINTER<char> buffer(ALLOC<char>(NET_OPT_DEFAULT_MAX_PACKET_SIZE + 1));
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

				if (Ws2_32::send(peer->pSocket, nullptr, NULL, MSG_NOSIGNAL) == SOCKET_ERROR)
				{
					ErasePeer(peer);
					LOG_ERROR(CSTRING("[%s] - Failed to send Package, reason: Socket Error"), SERVERNAME(this));
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
				res = Ws2_32::send(peer->pSocket, buffer.get(), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
					ErasePeer(peer);
					LOG_ERROR(CSTRING("[%s] - Failed to send Package, reason: Socket Error"), SERVERNAME(this));
					return WebServerHandshake::HandshakeRet_t::error;
				}

				size -= res;
			} while (size > 0);
		}

		buffer.free();

		// clear data
		peer->network.clear();

		const auto stringHost = CSTRING("Host");
		const auto stringOrigin = CSTRING("Origin");

		/* Handshake Failed - Display Error Message */
		if (Isset(NET_OPT_WS_CUSTOM_HANDSHAKE) ? GetOption<bool>(NET_OPT_WS_CUSTOM_HANDSHAKE) : NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE)
		{
			if (!(strcmp(reinterpret_cast<char*>(enc_Sec_Key), CSTRING("")) != 0 && strcmp(entries[stringHost].data(), host) == 0 && strcmp(entries[stringOrigin].data(), origin.get()) == 0))
			{
				origin.free();
				LOG_ERROR(CSTRING("[%s] - Handshake failed:\nReceived from Peer ('%s'):\nHost: %s\nOrigin: %s"), SERVERNAME(this), peer->IPAddr().get(), entries[stringHost].data(), entries[stringOrigin].data());
				return WebServerHandshake::HandshakeRet_t::missmatch;
			}
		}

		origin.free();
		FREE(enc_Sec_Key);
		return WebServerHandshake::HandshakeRet_t::success;
}

	LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen on Handshake"), SERVERNAME(this), peer->IPAddr().get());

	// clear data
	peer->network.clear();
	return WebServerHandshake::HandshakeRet_t::error;
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

	PEER_NOT_VALID_EX(peer, server,
		return NULL;
	);

	/* Handshake */
	if (!(server->Isset(NET_OPT_WS_NO_HANDSHAKE) ? server->GetOption<bool>(NET_OPT_WS_NO_HANDSHAKE) : NET_OPT_DEFAULT_WS_NO_HANDSHAKE))
	{
		do
		{
			PEER_NOT_VALID_EX(peer, server,
				return NULL;
			);

			const auto res = server->Handshake(peer);
			if (res == WebServerHandshake::peer_not_valid)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to invalid socket!"), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);
				return NULL;
			}
			if (res == WebServerHandshake::would_block)
			{
				continue;
			}
			if (res == WebServerHandshake::missmatch)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake missmatch!"), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);
				return NULL;
			}
			if (res == WebServerHandshake::error)
			{
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): dropped due to handshake error!"), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);
				return NULL;
			}
			if (res == WebServerHandshake::success)
			{
				peer->handshake = true;
				break;
			}
		} while (true);

		LOG_PEER(CSTRING("[%s] - Peer ('%s'): handshake has succesfully been performed"), SERVERNAME(server), peer->IPAddr().get());
	}
	peer->estabilished = true;
	server->OnPeerEstabilished(peer);

	while (peer)
	{
		if (!server->IsRunning())
			break;

		server->OnPeerUpdate(peer);

		SOCKET_VALID(peer->pSocket)
		{
			const auto restTime = server->DoReceive(peer);

#ifdef BUILD_LINUX
			usleep(restTime);
#else
			Kernel32::Sleep(restTime);
#endif

			continue;
		}

		break;
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
		// Set socket options
		for (const auto& entry : socketoption)
		{
			const auto res = Net::SetSocketOption(GetAcceptSocket(), entry);
			if (res < 0)
				LOG_ERROR(CSTRING("[%s] - Failure on settings socket option { 0x%ld : %i }"), SERVERNAME(this), entry.opt, LAST_ERROR);
		}

		const auto param = new Receive_t();
		param->server = this;
		param->peer = CreatePeer(client_addr, GetAcceptSocket());
		if (!param->peer) return;
		Thread::Create(Receive, param);
	}
}

void Server::DoSend(NET_PEER peer, const uint32_t id, NET_PACKAGE pkg, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	SOCKET_NOT_VALID(peer->pSocket) return;

	std::lock_guard<std::recursive_mutex> guard(peer->network._mutex_send);

	rapidjson::Document JsonBuffer;
	JsonBuffer.SetObject();
	rapidjson::Value key(CSTRING("CONTENT"), JsonBuffer.GetAllocator());
	JsonBuffer.AddMember(key, PKG.GetPackage(), JsonBuffer.GetAllocator());
	rapidjson::Value keyID(CSTRING("ID"), JsonBuffer.GetAllocator());
	rapidjson::Value bRaw(CSTRING("RAW"), JsonBuffer.GetAllocator());

	rapidjson::Value idValue;
	idValue.SetInt(id);
	JsonBuffer.AddMember(keyID, idValue, JsonBuffer.GetAllocator());
	JsonBuffer.AddMember(bRaw, false, JsonBuffer.GetAllocator());

	/* Ws2_32::buffer, later we cast to PBYTE */
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	JsonBuffer.Accept(writer);

	EncodeFrame((BYTE*)buffer.GetString(), buffer.GetSize(), peer, opc);
}

void Server::DoSend(NET_PEER peer, const uint32_t id, BYTE* data, size_t size, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	SOCKET_NOT_VALID(peer->pSocket) return;

	std::lock_guard<std::recursive_mutex> guard(peer->network._mutex_send);

	// write package id as big endian
	auto newBuffer = ALLOC<BYTE>(size + 5);
	newBuffer[0] = (id >> 24) & 0xFF;
	newBuffer[1] = (id >> 16) & 0xFF;
	newBuffer[2] = (id >> 8) & 0xFF;
	newBuffer[3] = (id) & 0xFF;
	memcpy(&newBuffer[4], data, size);
	newBuffer[size + 1] = '\0';

	EncodeFrame(newBuffer, size + 1, peer, opc);

	FREE(newBuffer);
}

void Server::EncodeFrame(BYTE* in_frame, const size_t frame_length, NET_PEER peer, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	SOCKET_NOT_VALID(peer->pSocket) return;

	/* ENCODE FRAME */
	auto frameCount = static_cast<int>(ceil((float)frame_length / NET_OPT_DEFAULT_MAX_PACKET_SIZE));
	if (frameCount == 0)
		frameCount = 1;

	const int lastFrameBufferLength = (frame_length % NET_OPT_DEFAULT_MAX_PACKET_SIZE) != 0 ? (frame_length % NET_OPT_DEFAULT_MAX_PACKET_SIZE) : (frame_length != 0 ? NET_OPT_DEFAULT_MAX_PACKET_SIZE : 0);

	size_t in_frame_offset = NULL;
	for (auto i = 0; i < frameCount; i++)
	{
		const unsigned char fin = (i != (frameCount - 1) ? 0 : NET_WS_FIN);
		const unsigned char opcode = (i != 0 ? NET_OPCODE_CONTINUE : opc);

		const size_t bufferLength = (i != (frameCount - 1) ? NET_OPT_DEFAULT_MAX_PACKET_SIZE : lastFrameBufferLength);
		CPOINTER<char> buf;
		size_t totalLength;

		if (bufferLength <= 125)
		{
			totalLength = bufferLength + 2;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = (char)bufferLength;
			memcpy(buf.get() + 2, &in_frame[in_frame_offset], bufferLength);
			in_frame_offset += bufferLength;
		}
		else if (bufferLength <= 65535)
		{
			totalLength = bufferLength + 4;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = NET_WS_PAYLOAD_LENGTH_16;
			buf.get()[2] = (bufferLength >> 8) & 0xFF;
			buf.get()[3] = (bufferLength) & 0xFF;
			memcpy(buf.get() + 4, &in_frame[in_frame_offset], bufferLength);
			in_frame_offset += bufferLength;
		}
		else
		{
			totalLength = bufferLength + 10;
			buf = ALLOC<char>(totalLength);
			buf.get()[0] = fin | opcode;
			buf.get()[1] = NET_WS_PAYLOAD_LENGTH_63;
			buf.get()[2] = (bufferLength >> 56) & 0xFF;
			buf.get()[3] = (bufferLength >> 48) & 0xFF;
			buf.get()[4] = (bufferLength >> 40) & 0xFF;
			buf.get()[5] = (bufferLength >> 32) & 0xFF;
			buf.get()[6] = (bufferLength >> 24) & 0xFF;
			buf.get()[7] = (bufferLength >> 16) & 0xFF;
			buf.get()[8] = (bufferLength >> 8) & 0xFF;
			buf.get()[9] = (bufferLength) & 0xFF;
			memcpy(buf.get() + 10, &in_frame[in_frame_offset], bufferLength);
			in_frame_offset += bufferLength;
		}

		if (!buf.valid())
		{
			LOG_DEBUG(CSTRING("[%s] - Buffer is nullptr!"), SERVERNAME(this));
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
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), SERVERNAME(this), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_WANT_CONNECT:
					case SSL_ERROR_WANT_ACCEPT:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), SERVERNAME(this), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_WANT_X509_LOOKUP:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), SERVERNAME(this), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_SYSCALL:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), SERVERNAME(this), peer->IPAddr().get());
						ErasePeer(peer);
						return;

					case SSL_ERROR_SSL:
						buf.free();
						return;

					case SSL_ERROR_WANT_WRITE:
						continue;

					default:
						buf.free();
						LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Send"), SERVERNAME(this), peer->IPAddr().get());
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
				const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<char*>(buf.get()), static_cast<int>(totalLength), MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK) continue;
					else {
						buf.free();
						ErasePeer(peer);
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK) continue;
					else {
						buf.free();
						ErasePeer(peer);
						LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}

				sendSize -= res;
			} while (sendSize > 0);
		}

		buf.free();
	}
	///////////////////////
}

DWORD Server::DoReceive(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return FREQUENZ(this);
	);

	if (peer->ssl)
	{
		// check socket still open
		if (Ws2_32::recv(peer->pSocket, nullptr, NULL, 0) == SOCKET_ERROR)
		{
#ifdef BUILD_LINUX
			if (errno != EWOULDBLOCK)
#else
			if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
			{
				ErasePeer(peer);

#ifdef BUILD_LINUX
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

				return FREQUENZ(this);
			}
		}

		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE);
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			switch (err)
			{
			case SSL_ERROR_ZERO_RETURN:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return FREQUENZ(this);

			case SSL_ERROR_WANT_CONNECT:
			case SSL_ERROR_WANT_ACCEPT:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return FREQUENZ(this);

			case SSL_ERROR_WANT_X509_LOOKUP:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return FREQUENZ(this);

			case SSL_ERROR_SYSCALL:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return FREQUENZ(this);

			case SSL_ERROR_SSL:
				/* Some servers did not close the connection properly */
				peer->network.reset();
				return FREQUENZ(this);

			case SSL_ERROR_WANT_READ:
				peer->network.reset();
				return FREQUENZ(this);

			default:
				peer->network.reset();
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): Something bad happen... on Receive"), SERVERNAME(this), peer->IPAddr().get());
				ErasePeer(peer);
				return FREQUENZ(this);
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
		const auto data_size = Ws2_32::recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
		if (data_size == SOCKET_ERROR)
		{
			peer->network.reset();

#ifdef BUILD_LINUX
			if (errno != EWOULDBLOCK)
#else
			if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
			{
				ErasePeer(peer);

#ifdef BUILD_LINUX
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				LOG_PEER(CSTRING("[%s] - Peer ('%s'): %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

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
	return NULL;
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
	const unsigned char FIN = peer->network.getData()[0] & NET_WS_FIN;
	const unsigned char OPC = peer->network.getData()[0] & NET_WS_OPCODE;
	if (OPC == NET_OPCODE_CLOSE)
	{
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been gracefully closed"), SERVERNAME(this), peer->IPAddr().get());
		ErasePeer(peer);
		return;
	}
	if (OPC == NET_OPCODE_PING)
	{
		Package pong;
		DoSend(peer, NET_WS_CONTROL_PACKAGE, pong, NET_OPCODE_PONG);
		peer->network.clear();
		return;
	}
	if (OPC == NET_OPCODE_PONG)
	{
		// todo work with timer
		peer->network.clear();
		return;
	}

	const auto IsMasked = peer->network.getData()[1] > NET_WS_MASK;
	unsigned int PayloadLength = peer->network.getData()[1] & NET_WS_PAYLOADLENGTH;
	auto NextBits = (16 / 8); // read the next 16 bits
	if (PayloadLength == 126)
	{
		const auto OffsetLen126 = (16 / 8);
		byte tmpRead[OffsetLen126] = {};
		memcpy(tmpRead, &peer->network.getData()[NextBits], OffsetLen126);
		PayloadLength = (unsigned int)Ws2_32::ntohs(*(u_short*)tmpRead);
		NextBits += OffsetLen126;
	}
	else if (PayloadLength == 127)
	{
		const auto OffsetLen127 = (64 / 8);
		byte tmpRead[OffsetLen127] = {};
		memcpy(tmpRead, &peer->network.getData()[NextBits], OffsetLen127);
#ifdef BUILD_LINUX
		PayloadLength = (unsigned int)be64toh(*(unsigned long long*)tmpRead);
#else
		PayloadLength = (unsigned int)Ws2_32::ntohll(*(unsigned long long*)tmpRead);
#endif
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
		LOG_PEER(CSTRING("[%s] - Peer ('%s'): connection has been closed due to unmasked message"), SERVERNAME(this), peer->IPAddr().get());
		ErasePeer(peer);
		return;
	}

	// frame is not complete
	if (!FIN)
	{
		if (OPC == NET_WS_CONTROLFRAME)
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
		if (OPC != NET_OPCODE_CONTINUE)
		{
			// this is a message frame process the message
			if (OPC == NET_OPCODE_TEXT || OPC == NET_OPCODE_BINARY)
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
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);
		return false;
	}

	if (!PKG.GetPackage().HasMember(CSTRING("CONTENT")))
	{
		LOG_PEER(CSTRING("Missing member 'CONTENT' in the package"));
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);
		return false;
	}

	Package Content;

	if (!PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.IsNull())
		Content.SetPackage(PKG.GetPackage().FindMember(CSTRING("CONTENT"))->value.GetObject());

	if (!CheckDataN(peer, id, Content))
	{
		if (!CheckData(peer, id, Content))
		{
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);
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

	LOG_DEBUG(CSTRING("[%s] - Peer ('%s'): timouted!"), SERVERNAME(this), peer->IPAddr().get());
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