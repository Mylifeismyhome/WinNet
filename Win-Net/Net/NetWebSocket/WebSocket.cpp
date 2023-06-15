/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "WebSocket.h"
#include <Net/Import/Kernel32.hpp>
#include <Net/Import/Ws2_32.hpp>

Net::WebSocket::IPRef::IPRef(const char* pointer)
{
	this->pointer = (char*)pointer;
}

Net::WebSocket::IPRef::~IPRef()
{
	FREE<char>(pointer);
}

const char* Net::WebSocket::IPRef::get() const
{
	return pointer;
}

Net::WebSocket::Server::Server()
{
	SetListenSocket(INVALID_SOCKET);
	SetAcceptSocket(INVALID_SOCKET);
	SetRunning(false);
	optionBitFlag = 0;
	socketOptionBitFlag = 0;
	hWorkThread = nullptr;
}

Net::WebSocket::Server::~Server()
{
	for (auto& entry : socketoption)
		FREE<SocketOptionInterface_t>(entry);

	socketoption.clear();

	for (auto& entry : option)
		FREE<OptionInterface_t>(entry);

	option.clear();
}

bool Net::WebSocket::Server::Isset(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return optionBitFlag & opt;
}

bool Net::WebSocket::Server::Isset_SocketOpt(const DWORD opt) const
{
	// use the bit flag to perform faster checks
	return socketOptionBitFlag & opt;
}

#pragma region Network Structure
void Net::WebSocket::Server::network_t::AllocReceiveBuffer(size_t size)
{
	if (_dataReceive.valid())
	{
		ClearReceiveBuffer();
	}

	_dataReceive = ALLOC<byte>(size + 1);
	memset(_dataReceive.get(), 0, size);
	_dataReceive.get()[size] = 0;

	_data_receive_size = size;
}

void Net::WebSocket::Server::network_t::ClearReceiveBuffer()
{
	if (_dataReceive.valid() == 0)
	{
		return;
	}

	_dataReceive.free();
	_data_receive_size = 0;
}

void Net::WebSocket::Server::network_t::ResetReceiveBuffer()
{
	if (_dataReceive.valid() == 0)
	{
		return;
	}

	memset(_dataReceive.get(), 0, _data_receive_size);
}

size_t Net::WebSocket::Server::network_t::GetReceiveBufferSize() const
{
	return _data_receive_size;
}

void Net::WebSocket::Server::network_t::setData(byte* pointer)
{
	deallocData();
	_data = pointer;
}

void Net::WebSocket::Server::network_t::setDataFragmented(byte* pointer)
{
	deallocDataFragmented();
	_dataFragment = pointer;
}

void Net::WebSocket::Server::network_t::allocData(const size_t size)
{
	clear();
	_data = ALLOC<byte>(size + 1);
	memset(getData(), NULL, size * sizeof(byte));
	getData()[size] = '\0';

	setDataSize(size);
}

void Net::WebSocket::Server::network_t::allocDataFragmented(const size_t size)
{
	clear();
	_dataFragment = ALLOC<byte>(size + 1);
	memset(getDataFragmented(), NULL, size * sizeof(byte));
	getDataFragmented()[size] = '\0';

	setDataSize(size);
}

void Net::WebSocket::Server::network_t::deallocData()
{
	_data.free();
}

void Net::WebSocket::Server::network_t::deallocDataFragmented()
{
	_dataFragment.free();

}
byte* Net::WebSocket::Server::network_t::getData() const
{
	return _data.get();
}

byte* Net::WebSocket::Server::network_t::getDataFragmented() const
{
	return _dataFragment.get();
}

void Net::WebSocket::Server::network_t::reset()
{
	ResetReceiveBuffer();
}

void Net::WebSocket::Server::network_t::clear()
{
	deallocData();
	deallocDataFragmented();
	_data_size = 0;
}

void Net::WebSocket::Server::network_t::setDataSize(const size_t size)
{
	_data_size = size;
}

size_t Net::WebSocket::Server::network_t::getDataSize() const
{
	return _data_size;
}

void Net::WebSocket::Server::network_t::setDataFragmentSize(const size_t size)
{
	_data_sizeFragment = size;
}

size_t Net::WebSocket::Server::network_t::getDataFragmentSize() const
{
	return _data_sizeFragment;
}

bool Net::WebSocket::Server::network_t::dataValid() const
{
	return _data.valid();
}

bool Net::WebSocket::Server::network_t::dataFragmentValid() const
{
	return _dataFragment.valid();
}

byte* Net::WebSocket::Server::network_t::getDataReceive()
{
	return _dataReceive.get();
}
#pragma endregion
NET_PEER Net::WebSocket::Server::CreatePeer(const sockaddr_in client_addr, const SOCKET socket)
{
	// UniqueID is equal to socket, since socket is already an unique ID
	const auto peer = ALLOC<Net::WebSocket::Server::peerInfo>();
	peer->UniqueID = socket;
	peer->pSocket = socket;
	peer->client_addr = client_addr;
	peer->ssl = nullptr;

	if (Net::SetDefaultSocketOption(socket, (Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE)) == 0)
	{
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => failed to apply default socket option for '%d'\n\tdiscarding socket..."), SERVERNAME(this), socket);
		return nullptr;
	}

	/*
	* Set/override socket options
	*/
	for (const auto& entry : socketoption)
	{
		const auto res = Ws2_32::setsockopt(peer->pSocket, entry->level, entry->opt, entry->value(), entry->optlen());
		if (res == SOCKET_ERROR)
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => failed to apply socket option { %i : %i } for socket '%d'"), SERVERNAME(this), entry->opt, LAST_ERROR, socket);
		}
	}

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
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => The TLS/SSL handshake was not successful but was shut down controlled and by the specifications of the TLS/SSL protocol"), SERVERNAME(this));
			return nullptr;
		}
		if (res < 0)
		{
			const auto err = SSL_get_error(peer->ssl, res);
			NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(err, true).c_str());
			ERR_clear_error();
			return nullptr;
		}
	}

	NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => New peer connected."), SERVERNAME(this), peer->IPAddr().get());

	// callback
	OnPeerConnect(peer);

	return peer;
}

bool Net::WebSocket::Server::ErasePeer(NET_PEER peer, bool clear)
{
	PEER_NOT_VALID(peer,
		return false;
	);

	if (clear)
	{
		// close endpoint
		SOCKET_VALID(peer->pSocket)
		{
			bool bBlocked = false;
			do
			{
				bBlocked = false;
				if (Ws2_32::closesocket(peer->pSocket) == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
#endif
					{
						bBlocked = true;
						continue;
					}
				}

			} while (bBlocked);

			peer->pSocket = INVALID_SOCKET;
		}

		// callback
#ifdef BUILD_LINUX
		OnPeerDisconnect(peer, errno);
#else
		OnPeerDisconnect(peer, Ws2_32::WSAGetLastError());
#endif

		NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => finished"), SERVERNAME(this), peer->IPAddr().get());

		peer->clear();

		return true;
	}

	peer->bErase = true;
	return true;
}

void Net::WebSocket::Server::peerInfo::clear()
{
	UniqueID = INVALID_UID;
	pSocket = INVALID_SOCKET;
	client_addr = sockaddr_in();
	estabilished = false;
	bErase = false;
	handshake = false;

	network.clear();
	network.reset();
	network.ClearReceiveBuffer();

	if (ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = nullptr;
	}
}

Net::WebSocket::IPRef Net::WebSocket::Server::peerInfo::IPAddr() const
{
	const auto buf = ALLOC<char>(INET_ADDRSTRLEN);
	return IPRef(Ws2_32::inet_ntop(AF_INET, &client_addr.sin_addr, buf, INET_ADDRSTRLEN));
}

void Net::WebSocket::Server::DisconnectPeer(NET_PEER peer, const int code)
{
	PEER_NOT_VALID(peer,
		return;
	);

	/*
	* NET_OPT_EXECUTE_PACKET_ASYNC allow packet execution in different threads
	* that threads might call DisconnectPeer
	* soo require a mutex to block it
	*/
	std::lock_guard<std::mutex> guard(peer->_mutex_disconnectPeer);

	if (code == 0)
	{
		NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => disconnected"), SERVERNAME(this), peer->IPAddr().get());
	}
	else
	{
		NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => disconnected due to the following reason '%s'"), SERVERNAME(this), peer->IPAddr().get(), Net::Codes::NetGetErrorMessage(code));
	}

	// now after we have sent him the reason, close connection
	ErasePeer(peer);
}

NET_THREAD(WorkThread)
{
	const auto server = (Net::WebSocket::Server*)parameter;
	if (server == nullptr)
	{
		return 0;
	}

	while (server->IsRunning())
	{
		/*
		* first accept new connections
		*/
		server->Acceptor();

		/*
		* then run tick
		*/
		server->Tick();

#ifdef BUILD_LINUX
		usleep(FREQUENZ(server) * 1000);
#else
		Kernel32::Sleep(FREQUENZ(server));
#endif
	}

	return 0;
}

void Net::WebSocket::Server::SetListenSocket(const SOCKET ListenSocket)
{
	this->ListenSocket = ListenSocket;
}

void Net::WebSocket::Server::SetAcceptSocket(const SOCKET AcceptSocket)
{
	this->AcceptSocket = AcceptSocket;
}

void Net::WebSocket::Server::SetRunning(const bool bRunning)
{
	this->bRunning = bRunning;
}

SOCKET Net::WebSocket::Server::GetListenSocket() const
{
	return ListenSocket;
}

SOCKET Net::WebSocket::Server::GetAcceptSocket() const
{
	return AcceptSocket;
}

bool Net::WebSocket::Server::IsRunning() const
{
	return bRunning;
}

#ifdef BUILD_LINUX
static void usleep_wrapper(DWORD duration)
{
	usleep(duration * 1000);
}
#endif

bool Net::WebSocket::Server::Run()
{
	if (IsRunning())
	{
		return false;
	}

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
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => ctx is NULL"), SERVERNAME(this));
			return false;
		}

		/* Set the key and cert */
		if (SSL_CTX_use_certificate_file(ctx, Isset(NET_OPT_SSL_CERT) ? GetOption<char*>(NET_OPT_SSL_CERT) : NET_OPT_DEFAULT_SSL_CERT, SSL_FILETYPE_PEM) <= 0)
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_CERT) ? GetOption<char*>(NET_OPT_SSL_CERT) : NET_OPT_DEFAULT_SSL_CERT);
			return false;
		}

		if (SSL_CTX_use_PrivateKey_file(ctx, Isset(NET_OPT_SSL_KEY) ? GetOption<char*>(NET_OPT_SSL_KEY) : NET_OPT_DEFAULT_SSL_KEY, SSL_FILETYPE_PEM) <= 0)
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_KEY) ? GetOption<char*>(NET_OPT_SSL_KEY) : NET_OPT_DEFAULT_SSL_KEY);
			return false;
		}

		/* load verfiy location (CA)*/
		NET_FILEMANAGER fmanager(Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
		if (!fmanager.file_exists())
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => File does not exits '%s'"), SERVERNAME(this), Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
			return false;
		}

		if (SSL_CTX_load_verify_locations(ctx, Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA, nullptr) <= 0)
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Failed to load %s"), SERVERNAME(this), Isset(NET_OPT_SSL_CA) ? GetOption<char*>(NET_OPT_SSL_CA) : NET_OPT_DEFAULT_SSL_CA);
			return false;
		}

		/* verify private key */
		if (!SSL_CTX_check_private_key(ctx))
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Private key does not match with the public certificate"), SERVERNAME(this));
			ERR_print_errors_fp(stderr);
			return false;
		}

		SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
		/*	SSL_CTX_set_info_callback(ctx, [](const SSL* ssl, int type, int value)
			{
					NET_LOG(CSTRING("CALLBACK CTX SET INFO!"));
			});*/

		NET_LOG_DEBUG(CSTRING("WinNet :: Server('%s') => Server is using method: %s"), SERVERNAME(this), Net::ssl::GET_SSL_METHOD_NAME(Isset(NET_OPT_SSL_METHOD) ? GetOption<int>(NET_OPT_SSL_METHOD) : NET_OPT_DEFAULT_SSL_METHOD).data());
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
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => WSAStartup has been failed with error: %d"), SERVERNAME(this), res);
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Could not find a usable version of Winsock.dll"), SERVERNAME(this));
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
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => getaddrinfo failed with error: %d"), SERVERNAME(this), res);
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Create a SOCKET for connecting to server
	SetListenSocket(Ws2_32::socket(result->ai_family, result->ai_socktype, result->ai_protocol));

	if (GetListenSocket() == INVALID_SOCKET) {
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => socket failed with error: %ld"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::freeaddrinfo(result);
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	if (Net::SetDefaultSocketOption(GetListenSocket(), (Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE)) == 0)
	{
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => failed to apply default socket option for '%d'\n\tdiscarding socket..."), SERVERNAME(this), GetListenSocket());
		Ws2_32::closesocket(GetListenSocket());
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	// Setup the TCP listening socket
	res = Ws2_32::bind(GetListenSocket(), result->ai_addr, static_cast<int>(result->ai_addrlen));

	if (res == SOCKET_ERROR) {
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => bind failed with error: %d"), SERVERNAME(this), LAST_ERROR);
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
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => listen failed with error: %d"), SERVERNAME(this), LAST_ERROR);
		Ws2_32::closesocket(GetListenSocket());
#ifndef BUILD_LINUX
		Ws2_32::WSACleanup();
#endif
		return false;
	}

	auto max_peers = Isset(NET_OPT_MAX_PEERS_THREAD) ? GetOption<size_t>(NET_OPT_MAX_PEERS_THREAD) : NET_OPT_DEFAULT_MAX_PEERS_THREAD;
	PeerPoolManager.set_max_peers(max_peers);

	PeerPoolManager.set_sleep_time(FREQUENZ(this));

#ifdef BUILD_LINUX
	PeerPoolManager.set_sleep_function(&usleep_wrapper);
#else
	PeerPoolManager.set_sleep_function(&Kernel32::Sleep);
#endif;

	hWorkThread = Net::Thread::Create(WorkThread, this);

	SetRunning(true);
	NET_LOG_SUCCESS(CSTRING("WinNet :: Server('%s') => started on Port: %d"), SERVERNAME(this), SERVERPORT(this));
	return true;
}

bool Net::WebSocket::Server::Close()
{
	if (!IsRunning())
	{
		NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Can't close server, because server is not running!"), SERVERNAME(this));
		return false;
	}

	if (hWorkThread)
	{
		Net::Thread::WaitObject(hWorkThread);
		Net::Thread::Close(hWorkThread);
		hWorkThread = nullptr;
	}

	SetRunning(false);

	if (GetListenSocket())
	{
		Ws2_32::closesocket(GetListenSocket());
	}

	if (GetAcceptSocket())
	{
		Ws2_32::closesocket(GetAcceptSocket());
	}

#ifndef BUILD_LINUX
	Ws2_32::WSACleanup();
#endif

	NET_LOG_DEBUG(CSTRING("WinNet :: Server('%s') => Closed!"), SERVERNAME(this));
	return true;
}

short Net::WebSocket::Server::Handshake(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return WebServerHandshake::HandshakeRet_t::peer_not_valid;
	);

	/* SSL */
	if (peer->ssl)
	{
		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), peer->network.GetReceiveBufferSize());
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			peer->network.reset();
			if (err != SSL_ERROR_SSL && err != SSL_ERROR_WANT_READ)
			{
				ErasePeer(peer);
				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(err, true).c_str());
				return WebServerHandshake::HandshakeRet_t::error;
			}

			return WebServerHandshake::HandshakeRet_t::would_block;
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
		const auto data_size = Ws2_32::recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), peer->network.GetReceiveBufferSize(), 0);
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
				if (errno != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

				return WebServerHandshake::HandshakeRet_t::error;
			}

			return WebServerHandshake::HandshakeRet_t::would_block;
		}

		// graceful disconnect
		if (data_size == 0)
		{
			peer->network.reset();
			ErasePeer(peer);
			NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Connection to peer closed gracefully."), SERVERNAME(this), peer->IPAddr().get());
			return WebServerHandshake::HandshakeRet_t::error;
		}

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
		const auto stringSecWebSocketKey = CSTRING("Sec-WebSocket-Key");

		unsigned int sha1 = 0;

		// Compute SHA1
		char enc_Sec_Key[NET_SHA1_BASE64_SIZE];
		NET_SHA1(entries[stringSecWebSocketKey].data()).add(CSTRING("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")).finalize().to_base64(enc_Sec_Key);

		char host[15];
		if (Isset(NET_OPT_WS_CUSTOM_HANDSHAKE) ? GetOption<bool>(NET_OPT_WS_CUSTOM_HANDSHAKE) : NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE)
			sprintf(host, CSTRING("%s:%i"), Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN, Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);
		else
			sprintf(host, CSTRING("127.0.0.1:%i"), Isset(NET_OPT_PORT) ? GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT);

		NET_CPOINTER<char> origin;
		if (Isset(NET_OPT_WS_CUSTOM_HANDSHAKE) ? GetOption<bool>(NET_OPT_WS_CUSTOM_HANDSHAKE) : NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE)
		{
			const auto originSize = strlen(Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN);
			origin = ALLOC<char>(originSize + 1);
			sprintf(origin.get(), CSTRING("%s%s"), (Isset(NET_OPT_SSL) ? GetOption<bool>(NET_OPT_SSL) : NET_OPT_DEFAULT_SSL) ? CSTRING("https://") : CSTRING("http://"), Isset(NET_OPT_WS_CUSTOM_ORIGIN) ? GetOption<char*>(NET_OPT_WS_CUSTOM_ORIGIN) : NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN);
		}

		// Create Response
		NET_CPOINTER<char> buffer(ALLOC<char>(NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE + 1));
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
				res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(buffer.get()), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
#endif
					else
					{
						ErasePeer(peer);

#ifdef BUILD_LINUX
						if (errno != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif
						return WebServerHandshake::HandshakeRet_t::error;
					}
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
				NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => Handshake failed:\nReceived from Peer ('%s'):\nHost: %s\nOrigin: %s"), SERVERNAME(this), peer->IPAddr().get(), entries[stringHost].data(), entries[stringOrigin].data());
				return WebServerHandshake::HandshakeRet_t::missmatch;
			}
		}

		origin.free();
		return WebServerHandshake::HandshakeRet_t::success;
	}

	NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => There was an problem encountered with the websocket handshake."), SERVERNAME(this), peer->IPAddr().get());

	// clear data
	peer->network.clear();
	return WebServerHandshake::HandshakeRet_t::error;
}

struct Receive_t
{
	Net::WebSocket::Server* server;
	NET_PEER peer;
};

Net::PeerPool::WorkStatus_t PeerWorker(void* pdata)
{
	const auto data = (Receive_t*)pdata;
	if (!data) return Net::PeerPool::WorkStatus_t::STOP;

	auto peer = data->peer;
	const auto server = data->server;

	if (!server->IsRunning()) return Net::PeerPool::WorkStatus_t::STOP;
	if (peer->bErase) return Net::PeerPool::WorkStatus_t::STOP;
	if (peer->pSocket == INVALID_SOCKET) return Net::PeerPool::WorkStatus_t::STOP;

	server->OnPeerUpdate(peer);

	/* Handshake */
	if (!(server->Isset(NET_OPT_WS_NO_HANDSHAKE) ? server->GetOption<bool>(NET_OPT_WS_NO_HANDSHAKE) : NET_OPT_DEFAULT_WS_NO_HANDSHAKE))
	{
		if (!peer->handshake)
		{
			const auto res = server->Handshake(peer);
			if (res == WebServerHandshake::peer_not_valid)
			{
				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => There was an error encountered with the peer. Connection to the peer will be dropped immediately."), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);

				FREE<Net::WebSocket::Server::peerInfo>(peer);
				return Net::PeerPool::WorkStatus_t::STOP;
			}
			if (res == WebServerHandshake::would_block)
			{
				return Net::PeerPool::WorkStatus_t::CONTINUE;
			}
			if (res == WebServerHandshake::missmatch)
			{
				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Missmatch in the WebSocket handshake. Connection to the peer will be dropped immediately."), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);

				return Net::PeerPool::WorkStatus_t::STOP;
			}
			if (res == WebServerHandshake::error)
			{
				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => There was an error on performing the WebSocket handshake. Connection to the peer will be dropped immediately."), SERVERNAME(server), peer->IPAddr().get());

				// erase him
				server->ErasePeer(peer, true);

				return Net::PeerPool::WorkStatus_t::STOP;
			}
			if (res == WebServerHandshake::success)
			{
				peer->handshake = true;
				peer->estabilished = true;
				server->OnPeerEstabilished(peer);

				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => WebSocket handshake with peer was successful."), SERVERNAME(server), peer->IPAddr().get());
			}

			return Net::PeerPool::WorkStatus_t::CONTINUE;
		}
	}

	return (!server->DoReceive(peer) ? Net::PeerPool::WorkStatus_t::FORWARD : Net::PeerPool::WorkStatus_t::CONTINUE);
}

void OnPeerDelete(void* pdata)
{
	const auto data = (Receive_t*)pdata;
	if (!data) return;

	auto peer = data->peer;
	const auto server = data->server;

	// erase him
	server->ErasePeer(peer, true);
}

void Net::WebSocket::Server::Acceptor()
{
	/* This function manages all the incomming connection */

	// if client waiting, accept the connection and save the socket
	auto client_addr = sockaddr_in();
	socklen_t slen = sizeof(client_addr);

	SOCKET accept_socket = Ws2_32::accept(GetListenSocket(), (sockaddr*)&client_addr, &slen);
	if (accept_socket == INVALID_SOCKET)
	{
#ifdef BUILD_LINUX
		if (errno != EWOULDBLOCK)
#else
		if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
		{
			NET_LOG_ERROR(CSTRING("WinNet :: Server('%s') => [accept] failed with error %d"), SERVERNAME(this), LAST_ERROR);
		}

		return;
	}

	auto peer = CreatePeer(client_addr, accept_socket);
	if (peer == nullptr)
	{
		return;
	}

	peer->network.AllocReceiveBuffer((Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE));

	const auto pdata = ALLOC<Receive_t>();
	pdata->server = this;
	pdata->peer = peer;

	/* add peer to peer thread pool */
	Net::PeerPool::peerInfo_t pInfo;
	pInfo.SetPeer(pdata);
	pInfo.SetWorker(&PeerWorker);
	pInfo.SetCallbackOnDelete(&OnPeerDelete);
	this->add_to_peer_threadpool(pInfo);
}

void Net::WebSocket::Server::DoSend(NET_PEER peer, const uint32_t id, NET_PACKET& pkg, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (peer->bErase)
	{
		return;
	}

	std::lock_guard<std::mutex> guard(peer->network._mutex_send);

	Net::Json::Document doc;
	doc[CSTRING("ID")] = static_cast<int>(id);
	doc[CSTRING("CONTENT")] = pkg.Data();
	doc[CSTRING("RAW")] = false;

	auto buffer = doc.Serialize(Net::Json::SerializeType::UNFORMATTED);

	auto dataBufferSize = buffer.size();
	NET_CPOINTER<BYTE> dataBuffer(ALLOC<BYTE>(dataBufferSize + 1));
	memcpy(dataBuffer.get(), buffer.get().get(), dataBufferSize);
	dataBuffer.get()[dataBufferSize] = '\0';
	buffer.clear();

	EncodeFrame(dataBuffer.reference().get(), dataBufferSize, peer, opc);

	dataBuffer.free();
}

void Net::WebSocket::Server::DoSend(NET_PEER peer, const uint32_t id, BYTE* data, size_t size, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (peer->bErase)
	{
		return;
	}

	std::lock_guard<std::mutex> guard(peer->network._mutex_send);

	// write packet id as big endian
	auto newBuffer = ALLOC<BYTE>(size + 5);
	newBuffer[0] = (id >> 24) & 0xFF;
	newBuffer[1] = (id >> 16) & 0xFF;
	newBuffer[2] = (id >> 8) & 0xFF;
	newBuffer[3] = (id) & 0xFF;
	memcpy(&newBuffer[4], data, size);
	newBuffer[size + 1] = '\0';

	EncodeFrame(newBuffer, size + 1, peer, opc);

	FREE<byte>(newBuffer);
}

void Net::WebSocket::Server::EncodeFrame(BYTE* in_frame, const size_t frame_length, NET_PEER peer, const unsigned char opc)
{
	PEER_NOT_VALID(peer,
		return;
	);

	SOCKET_NOT_VALID(peer->pSocket) return;

	/* ENCODE FRAME */
	const auto receive_buffer_size = (Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE);

	auto frameCount = static_cast<int>(ceil((float)frame_length / receive_buffer_size));
	if (frameCount == 0)
		frameCount = 1;

	const int lastFrameBufferLength = (frame_length % receive_buffer_size) != 0 ? (frame_length % receive_buffer_size) : (frame_length != 0 ? receive_buffer_size : 0);

	size_t in_frame_offset = 0;
	for (auto i = 0; i < frameCount; i++)
	{
		const unsigned char fin = (i != (frameCount - 1) ? 0 : NET_WS_FIN);
		const unsigned char opcode = (i != 0 ? NET_OPCODE_CONTINUE : opc);

		const size_t bufferLength = (i != (frameCount - 1) ? receive_buffer_size : lastFrameBufferLength);
		NET_CPOINTER<char> buf;
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

		if (!buf.valid()) return;

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
					if (err != SSL_ERROR_WANT_WRITE) buf.free();
					if (err != SSL_ERROR_SSL && err != SSL_ERROR_WANT_READ)
					{
						ErasePeer(peer);
						NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(err, true).c_str());
					}

					return;
				}
				ERR_clear_error();
			} while (res <= 0);
		}
		else
		{
			auto sendSize = totalLength;
			do
			{
				const auto res = Ws2_32::send(peer->pSocket, reinterpret_cast<const char*>(buf.get()), sendSize, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
					else
					{
						buf.free();
						ErasePeer(peer);
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						buf.free();
						ErasePeer(peer);
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
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

bool Net::WebSocket::Server::DoReceive(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return true;
	);

	if (peer->bErase)
		return true;

	if (peer->ssl)
	{
		const auto data_size = SSL_read(peer->ssl, reinterpret_cast<char*>(peer->network.getDataReceive()), peer->network.GetReceiveBufferSize());
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(peer->ssl, data_size);
			peer->network.reset();
			if (err != SSL_ERROR_SSL && err != SSL_ERROR_WANT_READ)
			{
				ErasePeer(peer);
				NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(err, true).c_str());
			}

			return true;
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
		const auto data_size = Ws2_32::recv(peer->pSocket, reinterpret_cast<char*>(peer->network.getDataReceive()), peer->network.GetReceiveBufferSize(), 0);
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
				if (errno != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(errno).c_str());
#else
				if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => %s"), SERVERNAME(this), peer->IPAddr().get(), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

				return true;
			}

			return true;
		}

		// graceful disconnect
		if (data_size == 0)
		{
			peer->network.reset();
			ErasePeer(peer);
			NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Connection to peer closed gracefully."), SERVERNAME(this), peer->IPAddr().get());
			return true;
		}

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
	return false;
}

void Net::WebSocket::Server::DecodeFrame(NET_PEER peer)
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

	// graceful disconnect
	if (OPC == NET_OPCODE_CLOSE)
	{
		ErasePeer(peer);
		NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Connection to peer closed gracefully."), SERVERNAME(this), peer->IPAddr().get());
		return;
	}
	if (OPC == NET_OPCODE_PING)
	{
		NET_PACKET pong;
		DoSend(peer, NET_WS_CONTROL_PACKET, pong, NET_OPCODE_PONG);
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
		NET_CPOINTER<byte> Payload(ALLOC<byte>(PayloadLength + 1));
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
		ErasePeer(peer);
		NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Connection to peer closed gracefully."), SERVERNAME(this), peer->IPAddr().get());
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
			NET_CPOINTER<byte> tmpDataFragement(ALLOC<byte>(dataFragementSize + 1));
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
					ProcessPacket(peer, peer->network.getData(), peer->network.getDataSize());
				else
				{
					ProcessPacket(peer, peer->network.getDataFragmented(), peer->network.getDataFragmentSize());
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

struct TPacketExcecute
{
	Net::Packet* m_packet;
	Net::WebSocket::Server* m_server;
	NET_PEER m_peer;
	int m_packetId;
};

NET_THREAD(ThreadPacketExecute)
{
	auto tpe = (TPacketExcecute*)parameter;
	if (!tpe)
	{
		return 0;
	}

	if (!tpe->m_server->CheckDataN(tpe->m_peer, tpe->m_packetId, *tpe->m_packet))
		if (!tpe->m_server->CheckData(tpe->m_peer, tpe->m_packetId, *tpe->m_packet))
			tpe->m_server->DisconnectPeer(tpe->m_peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);

	FREE<Net::Packet>(tpe->m_packet);
	FREE<TPacketExcecute>(tpe);
	return 0;
}

void Net::WebSocket::Server::ProcessPacket(NET_PEER peer, BYTE* data, const size_t size)
{
	PEER_NOT_VALID(peer,
		return;
	);

	if (!data)
	{
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DataInvalid);
		return;
	}

	NET_CPOINTER<Net::Packet> pPacket(ALLOC<Net::Packet>());
	if (!pPacket.valid())
	{
		DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DataInvalid);
		return;
	}

	/*
	* parse json
	* get packet id from it
	* and json content
	*
	* pass the json content into pPacket object
	*/
	int packetId = -1;
	{
		Net::Json::Document doc;
		if (!doc.Deserialize(reinterpret_cast<char*>(data)))
		{
			pPacket.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_DataInvalid);
			return;
		}

		if (!(doc[CSTRING("ID")] && doc[CSTRING("ID")]->is_int()))
		{
			pPacket.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_NoMemberID);
			return;
		}

		packetId = doc[CSTRING("ID")]->as_int();
		if (packetId < 0)
		{
			pPacket.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_MemberIDInvalid);
			return;
		}

		if (!(doc[CSTRING("CONTENT")] && doc[CSTRING("CONTENT")]->is_object())
			&& !(doc[CSTRING("CONTENT")] && doc[CSTRING("CONTENT")]->is_array()))
		{
			pPacket.free();
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_NoMemberContent);
			return;
		}

		if (doc[CSTRING("CONTENT")]->is_object())
		{
			pPacket.get()->Data().Set(doc[CSTRING("CONTENT")]->as_object());
		}
		else if (doc[CSTRING("CONTENT")]->is_array())
		{
			pPacket.get()->Data().Set(doc[CSTRING("CONTENT")]->as_array());
		}
	}

	/*
	* check for option async to execute the callback in a seperate thread
	*/
	if (Isset(NET_OPT_EXECUTE_PACKET_ASYNC) ? GetOption<bool>(NET_OPT_EXECUTE_PACKET_ASYNC) : NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC)
	{
		TPacketExcecute* tpe = ALLOC<TPacketExcecute>();
		tpe->m_packet = pPacket.get();
		tpe->m_server = this;
		tpe->m_peer = peer;
		tpe->m_packetId = packetId;
		const auto hThread = Net::Thread::Create(ThreadPacketExecute, tpe);
		if (hThread)
		{
			// Close only closes handle, it does not close the thread
			Net::Thread::Close(hThread);
			return;
		}

		FREE<TPacketExcecute>(tpe);
	}

	/*
	* execute in current thread
	*/
	if (!CheckDataN(peer, packetId, *pPacket.ref().get()))
		if (!CheckData(peer, packetId, *pPacket.ref().get()))
			DisconnectPeer(peer, NET_ERROR_CODE::NET_ERR_UndefinedFrame);

	pPacket.free();
}

void Net::WebSocket::Server::onSSLTimeout(NET_PEER peer)
{
	PEER_NOT_VALID(peer,
		return;
	);

	ErasePeer(peer);
	NET_LOG_PEER(CSTRING("WinNet :: Server('%s') '%s' => Peer timeout exceeded."), SERVERNAME(this), peer->IPAddr().get());
}

NET_NATIVE_PACKET_DEFINITION_BEGIN(Net::WebSocket::Server)
NET_PACKET_DEFINITION_END

void Net::WebSocket::Server::add_to_peer_threadpool(Net::PeerPool::peerInfo_t info)
{
	PeerPoolManager.add(info);
}

void Net::WebSocket::Server::add_to_peer_threadpool(Net::PeerPool::peerInfo_t* pinfo)
{
	PeerPoolManager.add(pinfo);
}

size_t Net::WebSocket::Server::count_peers_all()
{
	return PeerPoolManager.count_peers_all();
}

size_t Net::WebSocket::Server::count_peers(Net::PeerPool::peer_threadpool_t* pool)
{
	return PeerPoolManager.count_peers(pool);
}

size_t Net::WebSocket::Server::count_pools()
{
	return PeerPoolManager.count_pools();
}
