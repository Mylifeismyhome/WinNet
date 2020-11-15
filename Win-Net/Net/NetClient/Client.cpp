#include "Client.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Client)
Client::Client()
{
	Net::Codes::NetLoadErrorCodes();
	SetAllToDefault();
	NeedExit = FALSE;

	std::thread BaseTickThread(&Client::BaseTickThread, this);
	BaseTickThread.detach();
}

Client::~Client()
{
}

void Client::SetAllToDefault()
{
	SetSocket(INVALID_SOCKET);
	SetServerAddress(CSTRING(""));
	SetServerPort(NULL);
	SetConnected(false);
	SetKeysSet(NULL);
	sfrequenz = DEFAULT_FREQUENZ;
	sBlockingMode = DEFAULT_BLOCKING_MODE;
	sMaxThreads = DEFAULT_MAX_THREADS;
	sRSAKeySize = DEFAULT_RSA_KEY_SIZE;
	sAESKeySize = DEFAULT_AES_KEY_SIZE;
	sCryptPackage = DEFAULT_CRYPT_PACKAGES;
	sCompressPackage = DEFAULT_COMPRESS_PACKAGES;
	sCalcLatencyInterval = DEFAULT_CALC_LATENCY_INTERVAL;

	LOG_DEBUG(CSTRING("Refresh-Frequenz has been set to default value of %lld"), sfrequenz);
	LOG_DEBUG(CSTRING("Max Threads has been set to default value of %i"), sMaxThreads);
	LOG_DEBUG(CSTRING("RSA Key size has been set to default value of %llu"), sRSAKeySize);
	LOG_DEBUG(CSTRING("AES Key size has been set to default value of %llu"), sAESKeySize);
	LOG_DEBUG(CSTRING("Crypt Packages has been set to default value of %s"), sCryptPackage ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Compress Packages has been set to default value of %s"), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Blocking Mode has been set to default value of %s"), sBlockingMode ? CSTRING("enabled") : CSTRING("disabled"));
	LOG_DEBUG(CSTRING("Calculate latency interval has been set to default value of %i"), sCalcLatencyInterval);
}

/* Thread functions */
void Client::BaseTickThread()
{
	LOG_DEBUG(CSTRING("BaseTickThread() has been started!"));
	while (!DoNeedExit())
	{
		BaseTick();

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	}
	LOG_DEBUG(CSTRING("BaseTickThread() has been closed!"));
}

void Client::ReceiveThread()
{
	LOG_DEBUG(CSTRING("ReceiveThread() has been started!"));
	while (IsConnected())
	{
		Packager();

		std::this_thread::sleep_for(std::chrono::milliseconds(GetFrequenz()));
	}
	Clear();
	LOG_DEBUG(CSTRING("ReceiveThread() has been closed!"));
}

void Client::SetFrequenz(const long long sfrequenz)
{
	this->sfrequenz = sfrequenz;
	LOG_DEBUG(CSTRING("Refresh-Frequenz has been changed to %lld"), sfrequenz);
}

void Client::SetBlockingMode(const bool sBlockingMode)
{
	this->sBlockingMode = sBlockingMode;
	LOG_DEBUG(CSTRING("Blocking Mode has been %s"), sBlockingMode ? CSTRING("enabled") : CSTRING("disabled"));
}

void Client::SetMaxThreads(const u_short sMaxThreads)
{
	this->sMaxThreads = sMaxThreads;
	LOG_DEBUG(CSTRING("Max Threads has been changed to %i"), sMaxThreads);
}

void Client::SetRSAKeySize(const size_t sRSAKeySize)
{
	this->sRSAKeySize = sRSAKeySize;
	LOG_DEBUG(CSTRING("RSA Key size has been set to %llu"), sRSAKeySize);
}

void Client::SetAESKeySize(const size_t sAESKeySize)
{
	if (sAESKeySize != CryptoPP::AES::MIN_KEYLENGTH && sAESKeySize != CryptoPP::AES::KEYLENGTH_MULTIPLE && sAESKeySize != CryptoPP::AES::MAX_KEYLENGTH)
	{
		LOG_ERROR(CSTRING("AES Key size of %llu is not valid!"), sAESKeySize);
		return;
	}

	this->sAESKeySize = sAESKeySize;
	LOG_DEBUG(CSTRING("AES Key size has been set to %llu"), sAESKeySize);
}

void Client::SetCryptPackage(const bool sCryptPackage)
{
	this->sCryptPackage = sCryptPackage;
	LOG_DEBUG(CSTRING("Crypt Package has been %s"), sCryptPackage ? CSTRING("enabled") : CSTRING("disabled"));
}

void Client::SetCompressPackage(const bool sCompressPackage)
{
	this->sCompressPackage = sCompressPackage;
	LOG_DEBUG(CSTRING("Compress Package has been %s"), sCompressPackage ? CSTRING("enabled") : CSTRING("disabled"));
}

void Client::SetCalcLatencyInterval(const long sCalcLatencyInterval)
{
	this->sCalcLatencyInterval = sCalcLatencyInterval;
	LOG_DEBUG(CSTRING("Calculate latency interval has been set to %i"), sCalcLatencyInterval);
}

long long Client::GetFrequenz() const
{
	return sfrequenz;
}

bool Client::GetBlockingMode() const
{
	return sBlockingMode;
}

u_short Client::GetMaxThreads() const
{
	return sMaxThreads;
}

size_t Client::GetRSAKeySize() const
{
	return sRSAKeySize;
}

size_t Client::GetAESKeySize() const
{
	return sAESKeySize;
}

bool Client::GetCryptPackage() const
{
	return sCryptPackage;
}

bool Client::GetCompressPackage() const
{
	return sCompressPackage;
}

long Client::GetCalcLatencyInterval() const
{
	return sCalcLatencyInterval;
}

bool Client::ChangeMode(const bool blocking) const
{
	auto ret = true;
	u_long non_blocking = blocking ? 0 : 1;
	ret = NO_ERROR == ioctlsocket(GetSocket(), FIONBIO, &non_blocking);
	return ret;
}

bool Client::Connect(const char* Address, const u_short Port)
{
	if (IsConnected())
	{
		LOG_ERROR(CSTRING("Can't connect to server, reason: already connected!"));
		return false;
	}

	WSADATA wsaData;

	const auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		LOG_ERROR(CSTRING("[Client] - WSAStartup failed with error: %d"), iResult);
		return false;
	}

	SetServerAddress(Address);
	SetServerPort(Port);

	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char port_str[16] = {};
	sprintf_s(port_str, CSTRING("%hu"), Port);
	const auto host = getaddrinfo(Address, port_str, &hints, &connectSocketAddr);
	if (host != 0)
	{
		LOG_ERROR(CSTRING("[Client] - Could not look up host: %s:%d!"), GetServerAddress(), GetServerPort());
		return false;
	}

	for (auto addr = connectSocketAddr; addr != nullptr; addr = addr->ai_next)
	{
		SetSocket(socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol));
		if (GetSocket() == INVALID_SOCKET)
		{
			LOG_ERROR(CSTRING("[Client] - socket failed with error: %ld"), WSAGetLastError());
			WSACleanup();
			return false;
		}

		/* Connect to the server */
		if (connect(GetSocket(), addr->ai_addr, addr->ai_addrlen) == -1)
		{
			closesocket(GetSocket());
			SetSocket(INVALID_SOCKET);
		}
	}

	if (GetSocket() == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[Client] - failure on connecting to host: %s:%s"), GetServerAddress(), GetServerPort());
		return false;
	}

	// Set socket options
	for (const auto& entry : socketoption)
	{
		const auto res = _SetSocketOption(GetSocket(), entry);
		if (res < 0)
			LOG_ERROR(CSTRING("[Client] - Failure on settings socket option { 0x%ld : %i }"), entry.opt, GetLastError());
	}

	// successfully connected
	SetConnected(true);

	// Set non-blocking mode
	ChangeMode(GetBlockingMode());

	if (GetCryptPackage())
	{
		/* create RSA Key Pair */
		network.createNewRSAKeys(GetRSAKeySize());
	}

	// Create Receive Package Thread
	std::thread(&Client::ReceiveThread, this).detach();

	// callback
	OnConnected();

	return true;
}

bool Client::Disconnect()
{
	if (!IsConnected())
	{
		LOG_ERROR(CSTRING("Can't disconnect from server, reason: not connected!"));
		return false;
	}

	// connection has been closed
	ConnectionClosed();

	LOG_SUCCESS(CSTRING("Disconnected from server"));

	// callback
	OnDisconnected();
	return true;
}

void Client::Timeout()
{
	if (!IsConnected())
	{
		LOG_ERROR(CSTRING("Can't disconnect from server, reason: not connected!"));
		return;
	}

	// connection has been closed
	ConnectionClosed();

	LOG_ERROR(CSTRING("Connection has been closed, server did not answer anymore (TIMEOUT)"));

	// callback
	OnTimeout();
}

void Client::ConnectionClosed()
{
	if (GetSocket())
	{
		closesocket(GetSocket());
		SetSocket(INVALID_SOCKET);
	}

	network.latency = -1;

	SetConnected(false);
}

void Client::Terminate()
{
	if (IsConnected())
		Disconnect();

	NeedExit = TRUE;

	delete this;
}

void Client::Clear()
{
	if (IsConnected())
	{
		LOG_ERROR(CSTRING("Can not clear Client while being connected!"));
		return;
	}

	WSACleanup();

	network.clear();

	if (GetCryptPackage())
		SetKeysSet(false);
}

bool Client::DoNeedExit() const
{
	return NeedExit;
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

void Client::SetKeysSet(const bool KeysSet)
{
	this->KeysSet = KeysSet;
}

bool Client::GetKeysSet() const
{
	return KeysSet;
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
}

void Client::Network::AllocData(const size_t size)
{
	data = ALLOC<byte>(size + 1);
	memset(data.get(), NULL, size);
	data_size = 0;
	data_full_size = 0;
}

void Client::Network::clearData()
{
	data.free();
	data_size = 0;
	data_full_size = 0;
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

void Client::BaseTick()
{
	/* This function is used as an update function */
	if (IsConnected())
	{
		// Calculate latency interval
		if (network.lastCalcLatency < CURRENTCLOCKTIME)
		{
			std::thread(&Client::LatencyTick, this).detach();
			network.lastCalcLatency = CREATETIMER(GetCalcLatencyInterval());
		}
	}

	Tick();
}

void Client::LatencyTick()
{
	ICMP _icmp(GetServerAddress());
	_icmp.execute();

	network.latency = _icmp.getLatency();
}

void Client::SingleSend(const char* data, size_t size, bool& bPreviousSentFailed)
{
	if (!GetSocket())
		return;

	if (bPreviousSentFailed)
		return;

	do
	{
		const auto res = send(GetSocket(), data, static_cast<int>(size), 0);
		if (res == SOCKET_ERROR)
		{
			switch (WSAGetLastError())
			{
			case WSANOTINITIALISED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				LOG_PEER(CSTRING("Something bad happen... on Send"));
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
				LOG_PEER(CSTRING("A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				FREE(data);
				LOG_PEER(CSTRING("Something bad happen... on Send"));
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
				LOG_PEER(CSTRING("A successful WSAStartup() call must occur before using this function"));
				Disconnect();
				return;

			case WSAENETDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The network subsystem has failed"));
				Disconnect();
				return;

			case WSAEACCES:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				Disconnect();
				return;

			case WSAEINTR:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				Disconnect();
				return;

			case WSAEINPROGRESS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				Disconnect();
				return;

			case WSAEFAULT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The buf parameter is not completely contained in a valid part of the user address space"));
				Disconnect();
				return;

			case WSAENETRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				Disconnect();
				return;

			case WSAENOBUFS:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("No buffer space is available"));
				Disconnect();
				return;

			case WSAENOTCONN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The socket is not connected"));
				Disconnect();
				return;

			case WSAENOTSOCK:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The descriptor is not a socket"));
				Disconnect();
				return;

			case WSAEOPNOTSUPP:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				Disconnect();
				return;

			case WSAESHUTDOWN:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				Disconnect();
				return;

			case WSAEWOULDBLOCK:
				continue;

			case WSAEMSGSIZE:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				Disconnect();
				return;

			case WSAEHOSTUNREACH:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The remote host cannot be reached from this host at this time"));
				Disconnect();
				return;

			case WSAEINVAL:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				Disconnect();
				return;

			case WSAECONNABORTED:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAECONNRESET:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				Timeout();
				return;

			case WSAETIMEDOUT:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				Timeout();
				return;

			default:
				bPreviousSentFailed = true;
				data.free();
				LOG_PEER(CSTRING("Something bad happen... on Send"));
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
*			{KEY}{...}...								*						-
*			{IV}{...}...									*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...						*				{RAW DATA}{...}...
*			{DATA}{...}...								*				{DATA}{...}...
*	{END PACKAGE}									*		{END PACKAGE}
*
 */
void Client::DoSend(const int id, NET_PACKAGE pkg)
{
	if (!IsConnected())
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
	if (GetCryptPackage() && network.RSAHandshake)
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
			Disconnect();
			return;
		}

		if (!network.RSA)
		{
			Key.free();
			IV.free();

			LOG_ERROR(CSTRING("RSA Object has no instance"));
			Disconnect();
			return;
		}

		/* Encrypt AES Keypair using RSA */
		auto KeySize = GetAESKeySize();
		if (!network.RSA->encryptBase64(Key.reference().get(), KeySize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed Key to encrypt and encode to base64"));
			Disconnect();
			return;
		}

		size_t IVSize = CryptoPP::AES::BLOCKSIZE;
		if (!network.RSA->encryptBase64(IV.reference().get(), IVSize))
		{
			Key.free();
			IV.free();
			LOG_ERROR(CSTRING("Failed IV to encrypt and encode to base64"));
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

		if (GetCompressPackage())
			CompressData(dataBuffer.reference().get(), dataBufferSize);

		if (PKG.HasRawData())
		{
			const auto rawData = PKG.GetRawData();
			for (auto& data : rawData)
				aes.encrypt(data.value(), data.size());
		}

		combinedSize = dataBufferSize + strlen(NET_PACKAGE_HEADER) + strlen(NET_PACKAGE_SIZE) + strlen(NET_DATA) + strlen(NET_PACKAGE_FOOTER) + strlen(NET_AES_KEY) + strlen(NET_AES_IV) + KeySize + IVSize + 8;

		// Append Raw data package size
		if (PKG.HasRawData())
		{
			if (GetCompressPackage())
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.value(), data.size());
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
		SingleSend(Key, KeySize, bPreviousSentFailed);

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
		if (GetCompressPackage())
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
			if (GetCompressPackage())
			{
				const auto rawData = PKG.GetRawData();
				for (auto data : rawData)
					CompressData(data.value(), data.size());
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

		if (GetCompressPackage())
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
*			{KEY}{...}...								*						-
*			{IV}{...}...									*						-
*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
*			{RAW DATA}{...}...						*				{RAW DATA}{...}...
*			{DATA}{...}...								*				{DATA}{...}...
*	{END PACKAGE}									*		{END PACKAGE}
*
 */
void Client::DoReceive()
{
	if (!IsConnected())
		return;

	const auto data_size = recv(GetSocket(), reinterpret_cast<char*>(network.dataReceive), DEFAULT_MAX_PACKET_SIZE, 0);
	if (data_size == SOCKET_ERROR)
	{
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("A successful WSAStartup() call must occur before using this function"));
			Disconnect();
			return;

		case WSAENETDOWN:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The network subsystem has failed"));
			Disconnect();
			return;

		case WSAEFAULT:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The buf parameter is not completely contained in a valid part of the user address space"));
			Disconnect();
			return;

		case WSAENOTCONN:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The socket is not connected"));
			Disconnect();
			return;

		case WSAEINTR:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The (blocking) call was canceled through WSACancelBlockingCall()"));
			Disconnect();
			return;

		case WSAEINPROGRESS:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"));
			Disconnect();
			return;

		case WSAENETRESET:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"));
			Disconnect();
			return;

		case WSAENOTSOCK:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The descriptor is not a socket"));
			Disconnect();
			return;

		case WSAEOPNOTSUPP:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"));
			Disconnect();
			return;

		case WSAESHUTDOWN:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"));
			Disconnect();
			return;

		case WSAEWOULDBLOCK:
			ProcessPackages();
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			return;

		case WSAEMSGSIZE:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The message was too large to fit into the specified buffer and was truncated"));
			Disconnect();
			return;

		case WSAEINVAL:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"));
			Disconnect();
			return;

		case WSAECONNABORTED:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
			Timeout();
			return;

		case WSAETIMEDOUT:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The connection has been dropped because of a network failure or because the peer system failed to respond"));
			Timeout();
			return;

		case WSAECONNRESET:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"));
			Timeout();
			return;

		default:
			memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("Something bad happen..."));
			Disconnect();
			return;
		}
	}
	if (data_size == 0)
	{
		memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
		LOG_PEER(CSTRING("Connection has been gracefully closed"));
		Disconnect();
		return;
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
		if (network.data_full_size > 0)
		{
			if (network.data_size + data_size > network.data_full_size)
			{
				network.data_full_size += data_size;

				/* store incomming */
				const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
				memcpy(newBuffer, network.data.get(), network.data_size);
				memcpy(&newBuffer[network.data_size], network.dataReceive, data_size);
				network.data_size += data_size;
				network.data = newBuffer; // pointer swap
			}
			else
			{
				memcpy(&network.data.get()[network.data_size], network.dataReceive, data_size);
				network.data_size += data_size;
			}
		}
		else
		{
			/* store incomming */
			const auto newBuffer = ALLOC<BYTE>(network.data_size + data_size + 1);
			memcpy(newBuffer, network.data.get(), network.data_size);
			memcpy(&newBuffer[network.data_size], network.dataReceive, data_size);
			network.data_size += data_size;
			network.data = newBuffer; // pointer swap
		}
	}

	// check if incomming is even valid
	if (memcmp(&network.data.get()[0], NET_PACKAGE_BRACKET_OPEN, 1) != 0)
	{
		LOG_ERROR(CSTRING("Incomming data does not match with the net protocol - 0x1"));
		Disconnect();
		memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
		return;
	}
	if (memcmp(&network.data.get()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
	{
		LOG_ERROR(CSTRING("Incomming data does not match with the net protocol - 0x2"));
		Disconnect();
		memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
		return;
	}

	// check if we have something to process already - since we have a fixed data receive size, should this be fine to process in everytick
	for (size_t it = DEFAULT_MAX_PACKET_SIZE - 1; it > 0; --it)
	{
		if (!memcmp(&network.dataReceive[it], NET_PACKAGE_BRACKET_OPEN, 1))
		{
			if (!memcmp(&network.dataReceive[it], NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER)))
			{
				memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
				ProcessPackages();
				return;
			}
		}
	}

	memset(network.dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);

	GetPackageDataSize();
}

void Client::GetPackageDataSize()
{
	if (!network.data_size
		|| network.data_size == INVALID_SIZE)
		return;

	if (network.data_full_size > 0)
		return;

	for (size_t it = 0; it < network.data_size; ++it)
	{
		if (!memcmp(&network.data.get()[it], NET_PACKAGE_BRACKET_OPEN, 1))
		{
			// find data full size
			if (!memcmp(&network.data.get()[it], NET_PACKAGE_SIZE, strlen(NET_PACKAGE_SIZE)))
			{
				const auto startPos = it + strlen(NET_PACKAGE_SIZE);
				size_t endPos = NULL;
				for (auto z = startPos; z < network.data_size; ++z)
				{
					if (!memcmp(&network.data.get()[z], NET_PACKAGE_BRACKET_CLOSE, 1))
					{
						endPos = z;
						break;
					}
				}

				if (!endPos)
					return;

				const auto size = endPos - startPos - 1;
				CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(size + 1));
				memcpy(dataSizeStr.get(), &network.data.get()[startPos + 1], size);
				dataSizeStr.get()[size] = '\0';
				const auto dataSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
				network.data_full_size = dataSize;
				dataSizeStr.free();

				const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
				memcpy(newBuffer, network.data.get(), network.data_size);
				newBuffer[network.data_full_size] = '\0';
				network.data = newBuffer; // pointer swap

				break;
			}
		}
	}
}

void Client::ProcessPackages()
{
	if (!network.data_size
		|| network.data_size == INVALID_SIZE)
		return;

	do
	{
		auto continuePackage = false;

		// search for footer
		for (auto it = network.data_size - 1; it > 0; --it)
		{
			if (!memcmp(&network.data.get()[it], NET_PACKAGE_BRACKET_OPEN, 1))
			{
				if (!memcmp(&network.data.get()[it], NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER)))
				{
					// check if we have a header
					auto idx = NULL;
					if (memcmp(&network.data.get()[0], NET_PACKAGE_HEADER, strlen(NET_PACKAGE_HEADER)) != 0)
					{
						LOG_ERROR(CSTRING("Package has no header"));
						network.clearData();
						return;
					}

					idx += static_cast<int>(strlen(NET_PACKAGE_HEADER)) + static_cast<int>(strlen(NET_PACKAGE_SIZE));

					// read entire Package size
					size_t entirePackageSize = NULL;
					size_t offsetBegin = NULL;
					{
						for (size_t y = idx; y < network.data_size; ++y)
						{
							if (!memcmp(&network.data.get()[y], NET_PACKAGE_BRACKET_CLOSE, 1))
							{
								offsetBegin = y;
								const auto size = y - idx - 1;
								CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(size + 1));
								memcpy(dataSizeStr.get(), &network.data.get()[idx + 1], size);
								dataSizeStr.get()[size] = '\0';
								entirePackageSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
								dataSizeStr.free();
								break;
							}
						}
					}

					// Execute the package
					ExecutePackage(entirePackageSize, offsetBegin);

					// re-alloc buffer
					const auto leftSize = static_cast<int>(network.data_size - entirePackageSize) > 0 ? network.data_size - entirePackageSize : INVALID_SIZE;
					if (leftSize != INVALID_SIZE
						&& leftSize > 0)
					{
						const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
						memcpy(leftBuffer, &network.data.get()[entirePackageSize], leftSize);
						leftBuffer[leftSize] = '\0';
						network.data = leftBuffer; // swap pointer
						network.data_size = leftSize;
						network.data_full_size = NULL;

						continuePackage = true;
					}
					else
					{
						network.data.free();
						network.data_size = NULL;
						network.data_full_size = NULL;
					}

					break;
				}
			}
		}

		if (!continuePackage)
			break;

	} while (true);
}

void Client::ExecutePackage(const size_t size, const size_t begin)
{
	if (memcmp(&network.data.get()[size - strlen(NET_PACKAGE_FOOTER)], NET_PACKAGE_FOOTER, strlen(NET_PACKAGE_FOOTER)) != 0)
	{
		LOG_ERROR(CSTRING("Package has no footer"));
		network.clearData();
		return;
	}

	CPOINTER<BYTE> data;
	std::vector<Package_RawData_t> rawData;

	/* Crypt */
	if (GetCryptPackage() && network.RSAHandshake)
	{
		auto offset = begin + 1;

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

			LOG_ERROR(CSTRING("RSA Object has no instance"));
			Disconnect();
			return;
		}

		if (!network.RSA->decryptBase64(AESKey.reference().get(), AESKeySize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed Key to decrypt and decode the base64"));
			Disconnect();
			return;
		}

		if (!network.RSA->decryptBase64(AESIV.reference().get(), AESIVSize))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed IV to decrypt and decode the base64"));
			Disconnect();
			return;
		}

		NET_AES aes;
		if (!aes.Init(reinterpret_cast<char*>(AESKey.get()), reinterpret_cast<char*>(AESIV.get())))
		{
			AESKey.free();
			AESIV.free();

			LOG_ERROR(CSTRING("Failed to Init AES [1]"));
			Disconnect();
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

					if (GetCompressPackage())
						DecompressData(entry.value(), entry.size());

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
			if (offset + strlen(NET_PACKAGE_FOOTER) == size)
				break;

		} while (true);
	}
	else
	{
		auto offset = begin + 1;

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

					if (GetCompressPackage())
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

				if (GetCompressPackage())
					DecompressData(data.reference().get(), packageSize);
			}

			// we have reached the end of reading
			if (offset + strlen(NET_PACKAGE_FOOTER) == size)
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

	if (!CheckDataN(id, Content))
	{
		if (!CheckData(id, Content))
		{
			LOG_PEER(CSTRING("Package is not defined!"));
		}
	}

	data.free();
}

void Client::CompressData(BYTE*& data, size_t& size) const
{
	/* Compression */
	if (GetCompressPackage())
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

void Client::DecompressData(BYTE*& data, size_t& size) const
{
	/* Compression */
	if (GetCompressPackage())
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
		LOG_DEBUG(CSTRING("Decompressed data from size %llu to %llu"), PrevSize, size);
#endif
	}
}

void Client::Packager()
{
	/* This function manages all the incomming packages */
	DoReceive();
}

NET_CLIENT_BEGIN_DATA_PACKAGE_NATIVE(Client)
NET_CLIENT_DEFINE_PACKAGE(RSAHandshake, NET_NATIVE_PACKAGE_ID::PKG_RSAHandshake)
NET_CLIENT_DEFINE_PACKAGE(VersionPackage, NET_NATIVE_PACKAGE_ID::PKG_VersionPackage)
NET_CLIENT_DEFINE_PACKAGE(EstabilishConnectionPackage, NET_NATIVE_PACKAGE_ID::PKG_EstabilishPackage)
NET_CLIENT_DEFINE_PACKAGE(ClosePackage, NET_NATIVE_PACKAGE_ID::PKG_ClosePackage)
NET_CLIENT_END_DATA_PACKAGE

NET_BEGIN_FNC_PKG(Client, RSAHandshake)
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[%s] - Client has already been estabilished, something went wrong!"), FUNCTION_NAME);
	return;
}
if (network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[%s] - Client has already done the RSA Handshake, something went wrong!"), FUNCTION_NAME);
	return;
}

NET_JOIN_PACKAGE(pkg, pkgRel);

const auto publicKey = pkgRel.String(CSTRING("PublicKey"));

if (!publicKey.valid())
{
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
// should not happen
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[%s] - Client has already been estabilished, something went wrong!"), FUNCTION_NAME);
	return;
}
if (GetCryptPackage() && !network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[%s] - Client has not done the RSA Handshake yet, something went wrong!"), FUNCTION_NAME);
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
// should not happen
if (network.estabilished)
{
	LOG_ERROR(CSTRING("[%s] - Client has already been estabilished, something went wrong!"), FUNCTION_NAME);
	return;
}
if (GetCryptPackage() && !network.RSAHandshake)
{
	LOG_ERROR(CSTRING("[%s] - Client has not done the RSA Handshake yet, something went wrong!"), FUNCTION_NAME);
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

LOG_SUCCESS(CSTRING("Connection has been closed by Server"));

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