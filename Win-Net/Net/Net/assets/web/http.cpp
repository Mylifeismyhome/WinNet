#include "http.h"
#include <Net/Import/Ws2_32.hpp>

NET_IGNORE_CONVERSION_NULL
void Net::Web::Head::Network::AllocData(const size_t size)
{
	data = ALLOC<byte>(size + 1);
	memset(data.get(), NULL, size);
	data_size = 0;
	data_full_size = 0;
}

void Net::Web::Head::Network::clearData()
{
	data.free();
	data_size = 0;
	data_full_size = 0;
}

Net::Web::HeaderData_t::HeaderData_t(const char* key, char* value, const size_t size)
{
	this->key = nullptr;
	this->value = nullptr;

	const auto keyLen = strlen(key);
	this->key = ALLOC<char>(keyLen + 1);
	memcpy(this->key, key, keyLen);
	this->key[keyLen] = '\0';

	if (!value)
		return;

	const auto dataSize = size == INVALID_SIZE ? strlen(value) : size;
	this->value = ALLOC<char>(dataSize + 1);
	memcpy(this->value, value, dataSize);
	this->value[dataSize] = '\0';
}

Net::Web::HeaderData_t::HeaderData_t(const char* key, const char* value, const size_t size)
{
	this->key = nullptr;
	this->value = nullptr;

	const auto keyLen = strlen(key);
	this->key = ALLOC<char>(keyLen + 1);
	memcpy(this->key, key, keyLen);
	this->key[keyLen] = '\0';

	if (!value)
		return;

	const auto dataSize = size == INVALID_SIZE ? strlen(value) : size;
	this->value = ALLOC<char>(dataSize + 1);
	memcpy(this->value, value, dataSize);
	this->value[dataSize] = '\0';
}

Net::Web::HeaderData_t::HeaderData_t(const char* key, unsigned char* value, const size_t size)
{
	this->key = nullptr;
	this->value = nullptr;

	const auto keyLen = strlen(key);
	this->key = ALLOC<char>(keyLen + 1);
	memcpy(this->key, key, keyLen);
	this->key[keyLen] = '\0';

	if (!value)
		return;

	const auto dataSize = size == INVALID_SIZE ? strlen(reinterpret_cast<char*>(value)) : size;
	this->value = ALLOC<char>(dataSize + 1);
	memcpy(this->value, value, dataSize);
	this->value[dataSize] = '\0';
}

void Net::Web::HeaderData_t::free()
{
	FREE(this->key);
	FREE(this->value);
}

Net::Web::Head::Head()
{
	protocol = std::string();
	url = std::string();
	path = std::string();
	port = 0;

	rawData = std::string();
	headContent = std::string();
	bodyContent = std::string();
	resultCode = -1;

	connectSocket = SOCKET();
	connectSocketAddr = nullptr;

	// Default Header
	AddHeader(CSTRING("Content-Type"), CSTRING("application/x-www-form-urlencoded"));
	AddHeader(CSTRING("User-Agent"), NET_USER_AGENT);
}

Net::Web::Head::~Head()
{
	closesocket(connectSocket);

	INT_Parameters.clear();
	LONG_Parameters.clear();
	LONGLONG_Parameters.clear();
	FLOAT_Parameters.clear();
	STRING_Parameters.clear();

	// append header data
	for (auto& entry : headerData)
		entry.free();

	headerData.clear();
}

SOCKET Net::Web::Head::GetSocket() const
{
	return connectSocket;
}

std::string& Net::Web::Head::GetProtocol()
{
	return protocol;
}

std::string& Net::Web::Head::GetURL()
{
	return url;
}

std::string& Net::Web::Head::GetPath()
{
	return path;
}

short Net::Web::Head::GetPort() const
{
	return port;
}

void Net::Web::Head::SetResultCode(const int code)
{
	resultCode = code;
}

int Net::Web::Head::GetResultCode() const
{
	return resultCode;
}

void Net::Web::Head::SetRawData(std::string raw)
{
	rawData = raw;
}

std::string& Net::Web::Head::GetRawData()
{
	return rawData;
}

void Net::Web::Head::SetHeaderContent(std::string head)
{
	headContent = head;
}

std::string& Net::Web::Head::GetHeaderContent()
{
	return headContent;
}

void Net::Web::Head::SetBodyContent(std::string body)
{
	bodyContent = body;
}

std::string& Net::Web::Head::GetBodyContent()
{
	return bodyContent;
}

static bool IsSpecial(const char c)
{
	return (c == ' ' || c == '\\' || c == '<' || c == '>' || c == '{' || c == '}' || c == '?' || c == '/' || c == '#' || c == ':');
}

void Net::Web::Head::URL_Encode(char*& buffer) const
{
	// Convert to Hex
	static const auto lut = CSTRING("0123456789ABCDEF");
	const auto len = strlen(buffer);

	std::string hex;
	hex.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = buffer[i];
		if (!IsSpecial(c))
		{
			hex.push_back(c);
			continue;
		}

		hex.push_back('%');
		hex.push_back(lut[c >> 4]);
		hex.push_back(lut[c & 15]);
	}

	FREE(buffer);
	buffer = ALLOC<char>(hex.size() + 1);
	memcpy(buffer, hex.data(), hex.size());
}

void Net::Web::Head::URL_Encode(std::string& buffer) const
{
	// Convert to Hex
	static const auto lut = CSTRING("0123456789ABCDEF");
	const auto len = buffer.length();

	std::string hex;
	hex.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = buffer[i];
		if (!IsSpecial(c))
		{
			hex.push_back(c);
			continue;
		}

		hex.push_back('%');
		hex.push_back(lut[c >> 4]);
		hex.push_back(lut[c & 15]);
	}
	buffer = hex;
}

void Net::Web::Head::URL_Decode(char*& buffer) const
{
	// Convert to ASCII
	static const auto lut = CSTRING("0123456789ABCDEF");
	const auto len = strlen(buffer);
	std::string ascii;
	ascii.reserve(len / 2);
	for (size_t i = 0; i < len; i += 2)
	{
		auto step = 1;
		auto a = buffer[i];
		if (a == '%')
		{
			a = buffer[i + 1];
			step = 2;
		}
		const auto p = std::lower_bound(lut, lut + 16, a);

		auto b = buffer[i + step];
		const auto q = std::lower_bound(lut, lut + 16, b);

		if (step == 2)
		{
			ascii.push_back((char)(((p - lut) << 4) | (q - lut)));
			i++;
			continue;
		}

		if (buffer[i - 1] == '%')
		{
			ascii.push_back((char)(((p - lut) << 4) | (q - lut)));
		}
		else
		{
			if (a != '%')
				ascii.push_back(a);

			if (b != '%')
				ascii.push_back(b);
		}
	}

	FREE(buffer);
	buffer = ALLOC<char>(ascii.size() + 1);
	memcpy(buffer, ascii.data(), ascii.size());
}

void Net::Web::Head::URL_Decode(std::string& buffer) const
{
	// Convert to ASCII
	static const auto lut = CSTRING("0123456789ABCDEF");
	const auto len = buffer.length();
	std::string ascii;
	ascii.reserve(len / 2);
	for (size_t i = 0; i < len; i += 2)
	{
		auto step = 1;
		auto a = buffer[i];
		if (a == '%')
		{
			a = buffer[i + 1];
			step = 2;
		}
		const auto p = std::lower_bound(lut, lut + 16, a);

		auto b = buffer[i + step];
		const auto q = std::lower_bound(lut, lut + 16, b);

		if (step == 2)
		{
			ascii.push_back((char)(((p - lut) << 4) | (q - lut)));
			i++;
			continue;
		}

		if (buffer[i - 1] == '%')
		{
			ascii.push_back((char)(((p - lut) << 4) | (q - lut)));
		}
		else
		{
			if (a != '%')
				ascii.push_back(a);

			if (b != '%')
				ascii.push_back(b);
		}
	}
	buffer = ascii;
}

void Net::Web::Head::AddHeader(const char* key, char* value, const size_t size)
{
	// rewrite existening data
	for(auto& entry : headerData)
	{
		if(!strcmp(key, entry.key))
		{
			entry.free();
			entry = HeaderData_t(key, value, size);
			return;
		}
	}

	headerData.emplace_back(HeaderData_t(key, value, size));
}

void Net::Web::Head::AddHeader(const char* key, const char* value, const size_t size)
{
	// rewrite existening data
	for (auto& entry : headerData)
	{
		if (!strcmp(key, entry.key))
		{
			entry.free();
			entry = HeaderData_t(key, value, size);
			return;
		}
	}

	headerData.emplace_back(HeaderData_t(key, value, size));
}

void Net::Web::Head::AddHeader(const char* key, unsigned char* value, const size_t size)
{
	// rewrite existening data
	for (auto& entry : headerData)
	{
		if (!strcmp(key, entry.key))
		{
			entry.free();
			entry = HeaderData_t(key, value, size);
			return;
		}
	}

	headerData.emplace_back(HeaderData_t(key, value, size));
}

void Net::Web::Head::AddParam(const char* tag, int value)
{
	INT_Parameters.insert(std::make_pair(tag, value));
}

void Net::Web::Head::AddParam(const char* tag, long value)
{
	LONG_Parameters.insert(std::make_pair(tag, value));
}

void Net::Web::Head::AddParam(const char* tag, long long value)
{
	LONGLONG_Parameters.insert(std::make_pair(tag, value));
}

void Net::Web::Head::AddParam(const char* tag, float value)
{
	FLOAT_Parameters.insert(std::make_pair(tag, value));
}

void Net::Web::Head::AddParam(const char* tag, const char* value)
{
	STRING_Parameters.insert(std::make_pair(tag, value));
}

void Net::Web::Head::SetJSON(const char* json)
{
	STRING_JSON = json;
}

std::string Net::Web::Head::GetParameters() const
{
	// we can only choose between sending JSON or regular HTTP Style POST/GET
	if (!STRING_JSON.empty()) return STRING_JSON;

	// Add all parameters
	std::string params;
	for (auto it = INT_Parameters.rbegin(); it != INT_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str());
		params.append(CSTRING("&"));
	}

	for (auto it = LONG_Parameters.rbegin(); it != LONG_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str());
		params.append(CSTRING("&"));
	}

	for (auto it = LONGLONG_Parameters.rbegin(); it != LONGLONG_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str());
		params.append(CSTRING("&"));
	}

	for (auto it = FLOAT_Parameters.rbegin(); it != FLOAT_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str());
		params.append(CSTRING("&"));
	}

	for (auto it = STRING_Parameters.rbegin(); it != STRING_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str());
		params.append(CSTRING("&"));
	}

	const auto deleteLastSplitPos = params.find_last_of(CSTRING("&"));
	const auto fixedparams = params.substr(0, deleteLastSplitPos);
	return fixedparams;
}

bool Net::Web::Head::ParseResult()
{
	if (!network.data.valid())
	{
		LOG_ERROR(CSTRING("[Head] - Failure on parsing result, buffer is nullptr"));
		return false;
	}

	const std::string result = reinterpret_cast<char*>(network.data.get());
	rawData = result;

	// Get Header Content
	const auto headerContentPos = result.find(CSTRING("\r\n\r\n"));
	if (headerContentPos != std::string::npos)
		SetHeaderContent(result.substr(0, headerContentPos));

	// Get Body Content
	const auto bodyContentPos = result.find(CSTRING("\r\n\r\n"));
	if (bodyContentPos != std::string::npos)
		SetBodyContent(result.substr(bodyContentPos + 4));

	// Get Result Code
	const auto resultCodePos = result.find_first_of(CSTRING("HTTP/1.1"));
	if (resultCodePos != std::string::npos)
	{
		const auto resultCode = result.substr(resultCodePos + 9, 3);
		if (!NET_STRING_IS_NUMBER(resultCode))
		{
			LOG_ERROR(CSTRING("[Head] - Result code is not a number!"));
			return false;
		}

		SetResultCode(std::stoi(resultCode));
	}

	return GetResultCode() == 200;
}

void Net::Web::Head::ShutdownSocket(int how) const
{
	const auto res = shutdown(GetSocket(), how);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[Head] - Failed to shutdown connection: %d"), LAST_ERROR);
		closesocket(GetSocket());
#ifndef BUILD_LINUX
		WSACleanup();
#endif
	}
}

Net::Web::HTTP::HTTP(const char* url)
{
	// prepare request
	Inited = Init(url);
}

Net::Web::HTTP::~HTTP()
{
#ifndef BUILD_LINUX
	WSACleanup();
#endif

	if (connectSocketAddr)
	{
		freeaddrinfo(connectSocketAddr);
		connectSocketAddr = nullptr;
	}
}

bool Net::Web::HTTP::Init(const char* curl)
{
	const auto fullURL = std::string(curl);

	const auto protoclPos = fullURL.find(CSTRING("://"));
	if (protoclPos == std::string::npos)
	{
		LOG_ERROR(CSTRING("[HTTP] - URL is invalid! (missing Protocol)"));
		return false;
	}
	protocol = fullURL.substr(0, protoclPos);

	const auto pathPos = fullURL.find('/', protoclPos + 3);
	if (pathPos == std::string::npos)
	{
		url = fullURL.substr(protoclPos + 3);
		path = '/';
	}
	else
	{
		url = fullURL.substr(protoclPos + 3, pathPos - protoclPos - 3);
		path = fullURL.substr(pathPos);
	}

	auto tmpport = std::string();
	const auto portPos = url.find(':');
	if (portPos != std::string::npos)
	{
		tmpport = url.substr(portPos + 1);
		url = url.substr(0, portPos);
	}

	port = 80;
	if (!tmpport.empty())
	{
		if (!NET_STRING_IS_NUMBER(tmpport))
		{
			LOG_ERROR(CSTRING("[HTTP] - Port is not a number!"));
			return false;
		}

		port = static_cast<short>(std::stoi(tmpport));
	}

#ifndef BUILD_LINUX
	WSADATA wsaData;
	const auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - WSAStartup failed with error: %d"), iResult);
		return false;
	}
#endif

	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char port_str[16] = {};
	sprintf(port_str, CSTRING("%hu"), GetPort());
	const auto host = getaddrinfo(url.data(), port_str, &hints, &connectSocketAddr);
	if (host != 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - Could not look up host: %s://%s%s:%i"), protocol.data(), url.data(), path.data(), port);
		return false;
	}

	return true;
}

bool Net::Web::HTTP::IsInited() const
{
	return Inited;
}

size_t Net::Web::HTTP::DoSend(std::string& buffer) const
{
	auto size = buffer.length();
	do
	{
		const auto res = Ws2_32::send(GetSocket(), buffer.data(), static_cast<int>(buffer.length()), 0);
		if (res == SOCKET_ERROR)
		{
			#ifdef BUILD_LINUX
			if(errno == EACCES)
			{
				LOG_PEER(CSTRING("[HTTP] - The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast addressdress"));
				return 0;
			}
			if(errno == EWOULDBLOCK)
				continue;
			if(errno == EALREADY)
			{
				LOG_PEER(CSTRING("[HTTP] - EALREADY"));
				return 0;
			}
			if(errno == EBADF)
			{
				LOG_PEER(CSTRING("[HTTP] - EBADF"));
				return 0;
			}
			if(errno == ECONNRESET)
			{
				LOG_PEER(CSTRING("[HTTP] - ECONNRESET"));
				return 0;
			}
			if(errno == EDESTADDRREQ)
			{
				LOG_PEER(CSTRING("[HTTP] - EDESTADDRREQ"));
				return 0;
			}
			if(errno == EFAULT)
			{
				LOG_PEER(CSTRING("[HTTP] - EFAULT"));
				return 0;
			}
			if(errno == EINTR)
			{
				LOG_PEER(CSTRING("[HTTP] - EINTR"));
				return 0;
			}
			if(errno == EINVAL)
			{
				LOG_PEER(CSTRING("[HTTP] - EINVAL"));
				return 0;
			}
			if(errno == EISCONN)
			{
				LOG_PEER(CSTRING("[HTTP] - EISCONN"));
				return 0;
			}
			if(errno == EMSGSIZE)
			{
				LOG_PEER(CSTRING("[HTTP] - EMSGSIZE"));
				return 0;
			}
			if(errno == ENOBUFS)
			{
				LOG_PEER(CSTRING("[HTTP] - ENOBUFS"));
				return 0;
			}
			if(errno == ENOMEM)
			{
				LOG_PEER(CSTRING("[HTTP] - ENOMEM"));
				return 0;
			}
			if(errno == ENOTCONN)
			{
				LOG_PEER(CSTRING("[HTTP] - ENOTCONN"));
				return 0;
			}
			if(errno == ENOTSOCK)
			{
				LOG_PEER(CSTRING("[HTTP] - ENOTSOCK"));
				return 0;
			}
			if(errno == EOPNOTSUPP)
			{
				LOG_PEER(CSTRING("[HTTP] - EOPNOTSUPP"));
				return 0;
			}
			if(errno == EPIPE)
			{
				LOG_PEER(CSTRING("[HTTP] - EPIPE"));
				return 0;
			}
			#else
			if (WSAGetLastError() == WSANOTINITIALISED)
			{
				LOG_PEER(CSTRING("[HTTP] - A successful WSAStartup() call must occur before using this function"));
				return 0;
			}
			if (WSAGetLastError() == WSAENETDOWN)
			{
				LOG_PEER(CSTRING("[HTTP] - The network subsystem has failed"));
				return 0;
			}
			if (WSAGetLastError() == WSAEACCES)
			{
				LOG_PEER(CSTRING("[HTTP] - The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt() with the SO_BROADCAST socket option to enable use of the broadcast address"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINTR)
			{
				LOG_PEER(CSTRING("[HTTP] - A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall()"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINPROGRESS)
			{
				LOG_PEER(CSTRING("[HTTP] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function"));
				return 0;
			}
			if (WSAGetLastError() == WSAEFAULT)
			{
				LOG_PEER(CSTRING("[HTTP] - The buf parameter is not completely contained in a valid part of the user address space"));
				return 0;
			}
			if (WSAGetLastError() == WSAENETRESET)
			{
				LOG_PEER(CSTRING("[HTTP] - The connection has been broken due to the keep - alive activity detecting a failure while the operation was in progress"));
				return 0;
			}
			if (WSAGetLastError() == WSAENOBUFS)
			{
				LOG_PEER(CSTRING("[HTTP] - No buffer space is available"));
				return 0;
			}
			if (WSAGetLastError() == WSAENOTCONN)
			{
				LOG_PEER(CSTRING("[HTTP] - The socket is not connected"));
				return 0;
			}
			if (WSAGetLastError() == WSAENOTSOCK)
			{
				LOG_PEER(CSTRING("[HTTP] - The descriptor is not a socket"));
				return 0;
			}
			if (WSAGetLastError() == WSAEOPNOTSUPP)
			{
				LOG_PEER(CSTRING("[HTTP] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations"));
				return 0;
			}
			if (WSAGetLastError() == WSAESHUTDOWN)
			{
				LOG_PEER(CSTRING("[HTTP] - The socket has been shut down; it is not possible to send on a socket after shutdown() has been invoked with how set to SD_SEND or SD_BOTH"));
				return 0;
			}
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			if (WSAGetLastError() == WSAEMSGSIZE)
			{
				LOG_PEER(CSTRING("[HTTP] - The socket is message oriented, and the message is larger than the maximum supported by the underlying transport"));
				return 0;
			}
			if (WSAGetLastError() == WSAEHOSTUNREACH)
			{
				LOG_PEER(CSTRING("[HTTP] - The remote host cannot be reached from this host at this time"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINVAL)
			{
				LOG_PEER(CSTRING("[HTTP] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled"));
				return 0;
			}
			if (WSAGetLastError() == WSAECONNABORTED)
			{
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				return 0;
			}
			if (WSAGetLastError() == WSAECONNRESET)
			{
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was reset by the remote side executing a hard or abortive close. For UDP sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a Port Unreachable ICMP packet. The application should close the socket as it is no longer usable"));
				return 0;
			}
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				LOG_PEER(CSTRING("[HTTP] - The connection has been dropped, because of a network failure or because the system on the other end went down without notice"));
				return 0;
			}
			#endif

			LOG_PEER(CSTRING("[HTTP] - Something bad happen... on Send"));
			return 0;
		}

		size -= res;
	} while (size > 0);

	return buffer.length();
}

size_t Net::Web::HTTP::DoReceive()
{
	size_t data_size = 0;
	do
	{
		if (network.data_full_size != 0 && network.data_size >= network.data_full_size)
			break;

		data_size = Ws2_32::recv(GetSocket(), reinterpret_cast<char*>(network.dataReceive), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
		if (data_size == SOCKET_ERROR)
		{
			#ifdef BUILD_LINUX
			switch (errno)
			{
			case EWOULDBLOCK:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);

                                // read until we have the Content-Length
                                if (network.data_full_size == 0)
                                {
                                        if (!network.data.valid()) continue;
                                        std::string tmpBuffer(reinterpret_cast<char*>(network.data.get()));
                                        if (tmpBuffer.empty()) continue;

                                        // get the header length
                                        const auto headerLength = tmpBuffer.find(CSTRING("\r\n\r\n"));
                                        if (headerLength != std::string::npos)
                                                continue;

                                        const auto cLPos = tmpBuffer.find(CSTRING("Content-Length:"));
                                        if (cLPos == std::string::npos)
                                                continue;

                                        const auto breakPos = tmpBuffer.find_first_of('\r', cLPos);
                                        if (breakPos == std::string::npos)
                                                continue;

                                        const auto cLength = tmpBuffer.substr(cLPos + sizeof("Content-Length:"), (breakPos - cLPos - sizeof("Content-Length:")));

                                        if (!NET_STRING_IS_NUMBER(cLength))
                                        {
                                                LOG_PEER(CSTRING("[HTTP] - Something bad happen on reading content-length"));
                                                return 0;
                                        }

                                        const auto contentLength = std::stoi(cLength);

                                        // re-alloc
                                        network.data_full_size = contentLength + headerLength;
                                        const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
                                        memcpy(newBuffer, network.data.get(), network.data_size);
                                        newBuffer[network.data_full_size] = '\0';
                                        network.data = newBuffer;
                                }

                                continue;

			case ECONNREFUSED:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - ECONNREFUSED"));
                                return 0;

			case EFAULT:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - EFAULT"));
                                return 0;

			case EINTR:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - EINTR"));
                                return 0;

			case EINVAL:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - EINVAL"));
                                return 0;

			case ENOMEM:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - ENOMEM"));
                                return 0;

			case ENOTCONN:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - ENOTCONN"));
                                return 0;

			case ENOTSOCK:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - ENOTSOCK"));
                                return 0;

			default:
                                memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
                                LOG_PEER(CSTRING("[HTTP] - Something bad happen..."));
                                return 0;
			}
			#else
			switch (Ws2_32::WSAGetLastError())
			{
			case WSANOTINITIALISED:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - A successful WSAStartup() call must occur before using this function"));
				return 0;

			case WSAENETDOWN:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The network subsystem has failed"));
				return 0;

			case WSAEFAULT:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The buf parameter is not completely contained in a valid part of the user address space"));
				return 0;

			case WSAENOTCONN:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The socket is not connected"));
				return 0;

			case WSAEINTR:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The (blocking) call was canceled through WSACancelBlockingCall()"));
				return 0;

			case WSAEINPROGRESS:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"));
				return 0;

			case WSAENETRESET:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"));
				return 0;

			case WSAENOTSOCK:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The descriptor is not a socket"));
				return 0;

			case WSAEOPNOTSUPP:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"));
				return 0;

			case WSAESHUTDOWN:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"));
				return 0;

			case WSAEWOULDBLOCK:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);

				// read until we have the Content-Length
				if (network.data_full_size == 0)
				{
					if (!network.data.valid()) continue;
					std::string tmpBuffer(reinterpret_cast<char*>(network.data.get()));
					if (tmpBuffer.empty()) continue;

					// get the header length
					const auto headerLength = tmpBuffer.find(CSTRING("\r\n\r\n"));
					if (headerLength != std::string::npos)
						continue;

					const auto cLPos = tmpBuffer.find(CSTRING("Content-Length:"));
					if (cLPos == std::string::npos)
						continue;

					const auto breakPos = tmpBuffer.find_first_of('\r', cLPos);
					if (breakPos == std::string::npos)
						continue;

					const auto cLength = tmpBuffer.substr(cLPos + sizeof("Content-Length:"), (breakPos - cLPos - sizeof("Content-Length:")));

					if (!NET_STRING_IS_NUMBER(cLength))
					{
						LOG_PEER(CSTRING("[HTTP] - Something bad happen on reading content-length"));
						return 0;
					}

					const auto contentLength = std::stoi(cLength);

					// re-alloc
					network.data_full_size = contentLength + headerLength;
					const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
					memcpy(newBuffer, network.data.get(), network.data_size);
					newBuffer[network.data_full_size] = '\0';
					network.data = newBuffer;
				}

				continue;

			case WSAEMSGSIZE:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The message was too large to fit into the specified buffer and was truncated"));
				return 0;

			case WSAEINVAL:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"));
				return 0;

			case WSAECONNABORTED:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				return 0;

			case WSAETIMEDOUT:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The connection has been dropped because of a network failure or because the peer system failed to respond"));
				return 0;

			case WSAECONNRESET:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"));
				return 0;

			default:
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTP] - Something bad happen..."));
				return 0;
			}
			#endif
		}

		if (data_size == 0)
		{
			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			return network.data_size;
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
	} while (data_size > 0);
	return network.data_size;
}

bool Net::Web::HTTP::Get()
{
	if (!IsInited())
		return false;

	for (auto addr = connectSocketAddr; addr != nullptr; addr = addr->ai_next)
	{
		connectSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			LOG_ERROR(CSTRING("[HTTP] - socket failed with error: %ld"), LAST_ERROR);
#ifndef BUILD_LINUX
			WSACleanup();
#endif
			return false;
		}

		/* Connect to the server */
		if (connect(connectSocket, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == -1)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}
	}

	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTP] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// build request
	std::string req;
	req.append(CSTRING("GET "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Host: "));
	req.append(GetURL());
	// append header data
	for (const auto& entry : headerData)
	{
		req.append(CSTRING("\r\n"));
		req.append(entry.key);
		req.append(CSTRING(": "));
		req.append(entry.value);
	}
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket(SOCKET_WR);

		// Receive Response
		if (DoReceive() != 0)
		{
			// Parse the result
			const auto res = ParseResult();
			network.clearData();
			return res;
		}

		network.clearData();
		return false;
	}

	return false;
}

bool Net::Web::HTTP::Post()
{
	if (!IsInited())
		return false;

	for (auto addr = connectSocketAddr; addr != nullptr; addr = addr->ai_next)
	{
		connectSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			LOG_ERROR(CSTRING("[HTTP] - socket failed with error: %ld"), LAST_ERROR);
#ifndef BUILD_LINUX
			WSACleanup();
#endif
			return false;
		}

		/* Connect to the server */
		if (connect(connectSocket, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == -1)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}
	}

	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTP] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// Get Parameters
	auto params = GetParameters();

	// build request
	std::string req;
	req.append(CSTRING("POST "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Host: "));
	req.append(GetURL());
	// append header data
	for (const auto& entry : headerData)
	{
		req.append(CSTRING("\r\n"));
		req.append(entry.key);
		req.append(CSTRING(": "));
		req.append(entry.value);
	}
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Content-Length: "));
	std::stringstream param_length;
	param_length << params.length();
	req.append(param_length.str());
	param_length.clear();

	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));

	// Params
	req.append(params);
	params.clear();

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket(SOCKET_WR);

		// Receive Response
		if (DoReceive() != 0)
		{
			// Parse the result
			const auto res = ParseResult();
			network.clearData();
			return res;
		}

		network.clearData();
		return false;
	}

	return false;
}

#ifndef VS13
Net::Web::HTTPS::HTTPS(const char* url, const  ssl::NET_SSL_METHOD METHOD)
{
	ctx = nullptr;
	ssl = nullptr;

	// prepare request
	Inited = Init(url, METHOD);
}

Net::Web::HTTPS::~HTTPS()
{
#ifndef BUILD_LINUX
	WSACleanup();
#endif

	if (ssl)
	{
		SSL_free(ssl);
		ssl = nullptr;
	}

	if (ctx)
	{
		SSL_CTX_free(ctx);
		ctx = nullptr;
	}

	if(connectSocketAddr)
	{
		freeaddrinfo(connectSocketAddr);
		connectSocketAddr = nullptr;
	}
}

bool Net::Web::HTTPS::Init(const char* curl, const  ssl::NET_SSL_METHOD METHOD)
{
	const auto fullURL = std::string(curl);

	/* Initialize SSL */
	SSL_library_init();
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();

	// create ctx
	if ((ctx = SSL_CTX_new(NET_CREATE_SSL_OBJECT(METHOD))) == nullptr)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failed on creating SSL_CTX Object!"));
		return false;
	}

	LOG_DEBUG(CSTRING("[HTTPS] - using %s Methode!"), GET_SSL_METHOD_NAME(METHOD).data());

	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

	const auto protoclPos = fullURL.find(CSTRING("://"));
	if (protoclPos == std::string::npos)
	{
		LOG_ERROR(CSTRING("[HTTPS] - URL is invalid! (missing Protocol)"));
		return false;
	}
	protocol = fullURL.substr(0, protoclPos);

	const auto pathPos = fullURL.find('/', protoclPos + 3);
	if (pathPos == std::string::npos)
	{
		url = fullURL.substr(protoclPos + 3);
		path = '/';
	}
	else
	{
		url = fullURL.substr(protoclPos + 3, pathPos - protoclPos - 3);
		path = fullURL.substr(pathPos);
	}

	auto tmpport = std::string();
	const auto portPos = url.find(':');
	if (portPos != std::string::npos)
	{
		tmpport = url.substr(portPos + 1);
		url = url.substr(0, portPos);
	}

	port = 443;
	if (!tmpport.empty())
	{
		if (!NET_STRING_IS_NUMBER(tmpport))
		{
			LOG_ERROR(CSTRING("[HTTPS] - Port is not a number!"));
			return false;
		}

		port = static_cast<short>(std::stoi(tmpport));
	}

#ifndef BUILD_LINUX
	WSADATA wsaData;
	const auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - WSAStartup failed with error: %d"), iResult);
		return false;
	}
#endif

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTPS] - socket failed with error: %ld"), LAST_ERROR);
#ifndef BUILD_LINUX
		WSACleanup();
#endif
		return false;
	}

	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char port_str[16] = {};
	sprintf(port_str, CSTRING("%hu"), GetPort());
	const auto host = getaddrinfo(url.data(), port_str, &hints, &connectSocketAddr);
	if (host != 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - Could not look up host: %s://%s%s:%i"), protocol.data(), url.data(), path.data(), port);
		return false;
	}

	return true;
}

bool Net::Web::HTTPS::IsInited() const
{
	return Inited;
}

size_t Net::Web::HTTPS::DoSend(std::string& buffer) const
{
	auto res = 0;
	do
	{
		res = SSL_write(ssl, buffer.data(), static_cast<int>(buffer.length()));
		if (res <= 0)
		{
			const auto err = SSL_get_error(ssl, res);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				LOG_DEBUG(CSTRING("[HTTPS] - The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"));
				return 0;
			}
			if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
			{
				LOG_DEBUG(CSTRING("[HTTPS] - The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"));
				return 0;
			}
			if (err == SSL_ERROR_WANT_X509_LOOKUP)
			{
				LOG_DEBUG(CSTRING("[HTTPS] - The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"));
				return 0;
			}
			if (err == SSL_ERROR_SYSCALL)
			{
				LOG_DEBUG(CSTRING("[HTTPS] - Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"));
				return 0;
			}
			if (err == SSL_ERROR_SSL)
				break;

			if (err == SSL_ERROR_WANT_WRITE)
				continue;

			LOG_DEBUG(CSTRING("[HTTPS] - Something bad happen... on Send"));
			return 0;
		}
	} while (res <= 0);

	return buffer.length();
}

size_t Net::Web::HTTPS::DoReceive()
{
	for (;;)
	{
		if (network.data_full_size != 0 && network.data_size >= network.data_full_size)
			break;

		const auto data_size = SSL_read(ssl, network.dataReceive, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
		if (data_size <= 0)
		{
			const auto err = SSL_get_error(ssl, data_size);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTPS] - The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"));
				break;
			}
			if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
			{
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTPS] - The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"));
				return 0;
			}
			if (err == SSL_ERROR_WANT_X509_LOOKUP)
			{
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTPS] - The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"));
				return 0;
			}
			if (err == SSL_ERROR_SYSCALL)
			{
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				LOG_PEER(CSTRING("[HTTPS] - Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"));
				return 0;
			}
			if (err == SSL_ERROR_SSL)
			{
				/* Some servers did not close the connection properly */
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				break;
			}
			if (err == SSL_ERROR_WANT_READ)
			{
				memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);

				// read until we have the Content-Length
				if (network.data_full_size == 0)
				{
					if (!network.data.valid()) continue;
					std::string tmpBuffer(reinterpret_cast<char*>(network.data.get()));
					if (tmpBuffer.empty()) continue;

					// get the header length
					const auto headerLength = tmpBuffer.find(CSTRING("\r\n\r\n"));
					if (headerLength != std::string::npos)
						continue;

					const auto cLPos = tmpBuffer.find(CSTRING("Content-Length:"));
					if (cLPos == std::string::npos)
						continue;

					const auto breakPos = tmpBuffer.find_first_of('\r', cLPos);
					if (breakPos == std::string::npos)
						continue;

					const auto cLength = tmpBuffer.substr(cLPos + sizeof("Content-Length:"), (breakPos - cLPos - sizeof("Content-Length:")));

					if (!NET_STRING_IS_NUMBER(cLength))
					{
						LOG_PEER(CSTRING("[HTTP] - Something bad happen on reading content-length"));
						return 0;
					}

					const auto contentLength = std::stoi(cLength);

					// re-alloc
					network.data_full_size = contentLength + headerLength;
					const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
					memcpy(newBuffer, network.data.get(), network.data_size);
					newBuffer[network.data_full_size] = '\0';
					network.data = newBuffer;
				}

				continue;
			}

			memset(network.dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			LOG_PEER(CSTRING("[HTTPS] - Something bad happen... on Receive"));
			return 0;
		}
		ERR_clear_error();

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
	}

	return network.data_size;
}

bool Net::Web::HTTPS::Get()
{
	if (!IsInited())
		return false;

	auto sslRet = 0;
	for (auto addr = connectSocketAddr; addr != nullptr; addr = addr->ai_next)
	{
		connectSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			LOG_ERROR(CSTRING("[HTTP] - socket failed with error: %ld"), LAST_ERROR);
#ifndef BUILD_LINUX
			WSACleanup();
#endif
			return false;
		}

		/* Connect to the server */
		if (connect(connectSocket, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == -1)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}

		if (connectSocket == INVALID_SOCKET)
			continue;

		/* Create a SSL object */
		if ((ssl = SSL_new(ctx)) == nullptr)
		{
			LOG_ERROR(CSTRING("[HTTPS] - failure on creating ssl object"));
			return false;
		}

		/* Attach SSL to the socket */
		SSL_set_fd(ssl, static_cast<int>(connectSocket));

		/* Connect to SSL on the server side */
		sslRet = SSL_connect(ssl);
		if (sslRet <= 0)
		{
			if (ssl)
			{
				SSL_shutdown(ssl);
				SSL_free(ssl);
				ssl = nullptr;
			}

			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}
	}

	if (sslRet <= 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting ssl object to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	if(connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	/* Set socket to non-blocking */
	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// build request
	std::string req;
	req.append(CSTRING("GET "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Host: "));
	req.append(GetURL());
	// append header data
	for (const auto& entry : headerData)
	{
		req.append(CSTRING("\r\n"));
		req.append(entry.key);
		req.append(CSTRING(": "));
		req.append(entry.value);
	}
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));

	// Send Get Request
	if (DoSend(req))
	{
		ShutdownSocket(SOCKET_WR);

		// Receive Response
		if (DoReceive() != 0)
		{
			// Parse the result
			const auto res = ParseResult();
			network.clearData();
			return res;
		}

		network.clearData();
		return false;
	}

	return false;
}

bool Net::Web::HTTPS::Post()
{
	if (!IsInited())
		return false;

	auto sslRet = 0;
	for (auto addr = connectSocketAddr; addr != nullptr; addr = addr->ai_next)
	{
		connectSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (connectSocket == INVALID_SOCKET)
		{
			LOG_ERROR(CSTRING("[HTTP] - socket failed with error: %ld"), LAST_ERROR);
#ifndef BUILD_LINUX
			WSACleanup();
#endif
			return false;
		}

		/* Connect to the server */
		if (connect(connectSocket, addr->ai_addr, static_cast<int>(addr->ai_addrlen)) == -1)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}

		if (connectSocket == INVALID_SOCKET)
			continue;

		/* Create a SSL object */
		if ((ssl = SSL_new(ctx)) == nullptr)
		{
			LOG_ERROR(CSTRING("[HTTPS] - failure on creating ssl object"));
			return false;
		}

		/* Attach SSL to the socket */
		SSL_set_fd(ssl, static_cast<int>(connectSocket));

		/* Connect to SSL on the server side */
		sslRet = SSL_connect(ssl);
		if (sslRet <= 0)
		{
			if (ssl)
			{
				SSL_shutdown(ssl);
				SSL_free(ssl);
				ssl = nullptr;
			}

			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
		}
	}

	if(sslRet <= 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting ssl object to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	/* Set socket to non-blocking */
	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// Get Parameters
	auto params = GetParameters();

	// build request
	std::string req;
	req.append(CSTRING("POST "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Host: "));
	req.append(GetURL());
	// append header data
	for (const auto& entry : headerData)
	{
		req.append(CSTRING("\r\n"));
		req.append(entry.key);
		req.append(CSTRING(": "));
		req.append(entry.value);
	}
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Content-Length: "));
	std::stringstream param_length;
	param_length << params.length();
	req.append(param_length.str());
	param_length.clear();

	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));

	// Params
	req.append(params);
	params.clear();

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket(SOCKET_WR);

		// Receive Response
		if (DoReceive() != 0)
		{
			// Parse the result
			const auto res = ParseResult();
			network.clearData();
			return res;
		}

		network.clearData();
		return false;
	}

	return false;
}
NET_POP
#endif
