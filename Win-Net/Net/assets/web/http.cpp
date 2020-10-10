#include "http.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(web)
HTTPHead::HTTPHead()
{
	BufferSize = 512; // default

	protocol = std::string();
	url = std::string();
	path = std::string();
	port = 0;

	rawData = std::string();
	headContent = std::string();
	bodyContent = std::string();
	resultCode = -1;

	connectSocket = SOCKET();
	connectSocketAddr = sockaddr_in();

	contentType = CSTRING("application/x-www-form-urlencoded");
}

HTTPHead::~HTTPHead()
{
	closesocket(connectSocket);

	INT_Parameters.clear();
	LONG_Parameters.clear();
	LONGLONG_Parameters.clear();
	FLOAT_Parameters.clear();
	STRING_Parameters.clear();
}

SOCKET HTTPHead::GetSocket() const
{
	return connectSocket;
}

void HTTPHead::SetBufferSize(const size_t size)
{
	BufferSize = size;
}

size_t HTTPHead::GetBufferSize() const
{
	return BufferSize;
}

std::string& HTTPHead::GetProtocol()
{
	return protocol;
}

std::string& HTTPHead::GetURL()
{
	return url;
}

std::string& HTTPHead::GetPath()
{
	return path;
}

short HTTPHead::GetPort() const
{
	return port;
}

void HTTPHead::SetResultCode(const int code)
{
	resultCode = code;
}

int HTTPHead::GetResultCode() const
{
	return resultCode;
}

void HTTPHead::SetContentType(const char* type)
{
	contentType = type;
}

void HTTPHead::SetContentType(std::string& type)
{
	contentType = type;
}

void HTTPHead::SetRawData(std::string& raw)
{
	rawData = raw;
}

std::string& HTTPHead::GetRawData()
{
	return rawData;
}

void HTTPHead::SetHeaderContent(std::string& head)
{
	headContent = head;
}

std::string& HTTPHead::GetHeaderContent()
{
	return headContent;
}

void HTTPHead::SetBodyContent(std::string& body)
{
	bodyContent = body;
}

std::string& HTTPHead::GetBodyContent()
{
	return bodyContent;
}

static bool IsSpecial(const char c)
{
	return (c == ' ' || c == '\\' || c == '<' || c == '>' || c == '{' || c == '}' || c == '?' || c == '/' || c == '#' || c == ':');
}

void HTTPHead::URL_Encode(char*& buffer) const
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

void HTTPHead::URL_Encode(std::string& buffer)
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

void HTTPHead::URL_Decode(char*& buffer) const
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

void HTTPHead::URL_Decode(std::string& buffer) const
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

void HTTPHead::AddParam(const char* tag, int value)
{
	INT_Parameters.insert(std::make_pair(tag, value));
}

void HTTPHead::AddParam(const char* tag, long value)
{
	LONG_Parameters.insert(std::make_pair(tag, value));
}

void HTTPHead::AddParam(const char* tag, long long value)
{
	LONGLONG_Parameters.insert(std::make_pair(tag, value));
}

void HTTPHead::AddParam(const char* tag, float value)
{
	FLOAT_Parameters.insert(std::make_pair(tag, value));
}

void HTTPHead::AddParam(const char* tag, const char* value)
{
	STRING_Parameters.insert(std::make_pair(tag, value));
}

void HTTPHead::AddJSON(const char* json)
{
	JSON_Parameters.emplace_back(json);
}

std::string HTTPHead::GetParameters() const
{
	// Add all parameters
	std::string params;
	for (auto it = INT_Parameters.rbegin(); it != INT_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str().c_str());
		params.append(CSTRING("&"));
	}

	for (auto it = LONG_Parameters.rbegin(); it != LONG_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str().c_str());
		params.append(CSTRING("&"));
	}

	for (auto it = LONGLONG_Parameters.rbegin(); it != LONGLONG_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str().c_str());
		params.append(CSTRING("&"));
	}

	for (auto it = FLOAT_Parameters.rbegin(); it != FLOAT_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str().c_str());
		params.append(CSTRING("&"));
	}

	for (auto it = STRING_Parameters.rbegin(); it != STRING_Parameters.rend(); ++it)
	{
		params.append(it->first);
		params.append(CSTRING("="));
		std::stringstream value;
		value << it->second;
		params.append(value.str().c_str());
		params.append(CSTRING("&"));
	}

	for(auto& entry : JSON_Parameters)
		params.append(entry.data());

	const auto deleteLastSplitPos = params.find_last_of(CSTRING("&"));
	const auto fixedparams = params.substr(0, deleteLastSplitPos);
	return fixedparams;
}

bool HTTPHead::ParseResult(byte*& buffer)
{
	if (!buffer)
	{
		LOG_ERROR(CSTRING("[HTTPHead] - Failure on parsing result, buffer is nullptr"));
		return false;
	}

	const std::string result = reinterpret_cast<char*>(buffer);
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
			LOG_ERROR(CSTRING("[HTTPHead] - Result code is not a number!"));
			return false;
		}

		SetResultCode(std::stoi(resultCode));
	}

	return GetResultCode() == 200;
}

void HTTPHead::ShutdownSocket() const
{
	const auto res = shutdown(GetSocket(), SD_SEND);
	if (res == SOCKET_ERROR)
	{
		LOG_ERROR(CSTRING("[HTTPHead] - Failed to shutdown connection: %d"), WSAGetLastError());
		closesocket(GetSocket());
		WSACleanup();
	}
}

HTTP::HTTP(std::string fullURL)
{
	// prepare request
	Inited = Init(fullURL);
}

HTTP::~HTTP()
{
	WSACleanup();
}

bool HTTP::Init(std::string& fullURL)
{
	const auto protoclPos = fullURL.find(CSTRING("://"));
	if (protoclPos == std::string::npos)
	{
		LOG_ERROR(CSTRING("[HTTP] - URL is invalid! (missing Protocol)"));
		return false;
	}
	protocol = fullURL.substr(0, protoclPos);
	/*if (protocol != CSTRING("http"))
	{
		LOG_ERROR(CSTRING("[HTTP] - Invalid Protocol, please use NET_HTTPS"));
		return false;
	}*/

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

		port = std::stoi(tmpport);
	}

	WSADATA wsaData;

	const auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - WSAStartup failed with error: %d"), iResult);
		return false;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTP] - socket failed with error: %ld"), WSAGetLastError());
		WSACleanup();
		return false;
	}

	const auto host = gethostbyname(url.data());
	if (!host)
	{
		LOG_ERROR(CSTRING("[HTTP] - Could not look up host: %s://%s%s:%i"), protocol.data(), url.data(), path.data(), port);
		return false;
	}

	// set SocketAddr
	auto addr = in_addr();
	memcpy(&addr, host->h_addr_list[0], sizeof(in_addr));
	memset(&connectSocketAddr, 0, sizeof(connectSocketAddr));
	connectSocketAddr.sin_family = AF_INET;
	connectSocketAddr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	connectSocketAddr.sin_port = htons(port);

	return true;
}

bool HTTP::IsInited() const
{
	return Inited;
}

size_t HTTP::DoSend(std::string& buffer) const
{
	auto size = buffer.length();
	do
	{
		const auto res = send(GetSocket(), buffer.data(), static_cast<int>(buffer.length()), 0);
		if (res == SOCKET_ERROR)
		{
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

			LOG_PEER(CSTRING("[HTTP] - Something bad happen... on Send"));
			return 0;
		}

		size -= res;
	} while (size > 0);

	return buffer.length();
}

size_t HTTP::DoReceive(byte*& buffer) const
{
	size_t data_size = 0;
	auto result = 0;
	do
	{
		auto tmpReceive = ALLOC<byte>(GetBufferSize() + 1);
		result = recv(GetSocket(), reinterpret_cast<char*>(tmpReceive), GetBufferSize(), 0);
		if (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSANOTINITIALISED)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - A successful WSAStartup() call must occur before using this function"));
				return 0;
			}
			if (WSAGetLastError() == WSAENETDOWN)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The network subsystem has failed"));
				return 0;
			}
			if (WSAGetLastError() == WSAEFAULT)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The buf parameter is not completely contained in a valid part of the user address space"));
				return 0;
			}
			if (WSAGetLastError() == WSAENOTCONN)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The socket is not connected"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINTR)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The (blocking) call was canceled through WSACancelBlockingCall()"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINPROGRESS)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"));
				return 0;
			}
			if (WSAGetLastError() == WSAENETRESET)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"));
				return 0;
			}
			if (WSAGetLastError() == WSAENOTSOCK)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The descriptor is not a socket"));
				return 0;
			}
			if (WSAGetLastError() == WSAEOPNOTSUPP)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"));
				return 0;
			}
			if (WSAGetLastError() == WSAESHUTDOWN)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"));
				return 0;
			}
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				FREE(tmpReceive);

				if (!buffer)
				{
					result = 1;
					continue;
				}

				std::string tmpBuffer = reinterpret_cast<char*>(buffer);
				const auto cLPos = tmpBuffer.find(CSTRING("Content-Length:"));
				if (cLPos == std::string::npos)
				{
					result = 1;
					continue;
				}

				const auto breakPos = tmpBuffer.find_first_of('\r', cLPos);
				if (breakPos == std::string::npos)
				{
					result = 1;
					continue;
				}

				const auto cLength = tmpBuffer.substr(cLPos + sizeof("Content-Length:"), (breakPos - cLPos - sizeof("Content-Length:")));

				if (!NET_STRING_IS_NUMBER(cLength))
				{
					LOG_PEER(CSTRING("[HTTP] - Something bad happen on reading content-length"));
					return 0;
				}

				const auto contentLength = std::stoi(cLength);
				if (data_size >= contentLength)
					break;

				result = 1;
				continue;
			}
			if (WSAGetLastError() == WSAEMSGSIZE)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The message was too large to fit into the specified buffer and was truncated"));
				return 0;
			}
			if (WSAGetLastError() == WSAEINVAL)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"));
				return 0;
			}
			if (WSAGetLastError() == WSAECONNABORTED)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
				return 0;
			}
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The connection has been dropped because of a network failure or because the peer system failed to respond"));
				return 0;
			}
			if (WSAGetLastError() == WSAECONNRESET)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTP] - The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"));
				return 0;
			}

			LOG_PEER(CSTRING("[HTTP] - Something bad happen... on Receive"));
			return 0;
		}
		tmpReceive[result] = '\0';

		if (!buffer)
		{
			buffer = ALLOC<byte>(result + 1);
			memcpy_s(buffer, result, tmpReceive, result);
			buffer[result] = '\0';
			data_size = result;
		}
		else
		{
			auto store = ALLOC<byte>(data_size + 1);
			memcpy_s(store, data_size, buffer, data_size);
			store[data_size] = '\0';
			FREE(buffer);
			buffer = ALLOC<byte>(data_size + result + 1);
			memcpy_s(buffer, (data_size + result), store, data_size);
			memcpy_s(&buffer[data_size], (data_size + result), tmpReceive, result);
			buffer[(data_size + result)] = '\0';
			FREE(store);
			data_size += result;
		}

		FREE(tmpReceive);
	} while (result > 0);
	return data_size;
}

bool HTTP::Get()
{
	if (!IsInited())
		return false;

	if (connect(GetSocket(), (sockaddr*)&connectSocketAddr, sizeof(connectSocketAddr)) < 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - failed to connect to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// Get Parameters
	auto params = GetParameters();

	// build request
	std::string req;
	req.append(CSTRING("GET "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\nHost: "));
	req.append(GetURL());
	req.append(CSTRING("\n\n"));

	// Params
	req.append(params);
	params.clear();

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket();

		// Receive Response
		byte* buffer = nullptr;
		if (DoReceive(buffer) != 0)
		{
			// Parse the result
			const auto res = ParseResult(buffer);
			FREE(buffer);
			return res;
		}

		FREE(buffer);
		return false;
	}

	return false;
}

bool HTTP::Post()
{
	if (!IsInited())
		return false;

	if (connect(GetSocket(), (sockaddr*)&connectSocketAddr, sizeof(connectSocketAddr)) < 0)
	{
		LOG_ERROR(CSTRING("[HTTP] - failed to connect to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
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
	req.append(CSTRING("\nHost: "));
	req.append(GetURL());
	req.append(CSTRING("\nContent-Length: "));
	std::stringstream param_length;
	param_length << params.length();
	req.append(param_length.str());
	param_length.clear();
	req.append(CSTRING("\nContent-Type: "));
	req.append(contentType.data());
	req.append(CSTRING("\n\n"));

	// Params
	req.append(params);
	params.clear();

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket();

		// Receive Response
		byte* buffer = nullptr;
		if (DoReceive(buffer) != 0)
		{
			// Parse the result
			const auto res = ParseResult(buffer);
			FREE(buffer);
			return res;
		}

		FREE(buffer);
		return false;
	}

	return false;
}

HTTPS::HTTPS(std::string fullURL, const NET_SSL_METHOD METHOD)
{
	ctx = nullptr;
	ssl = nullptr;

	// prepare request
	Inited = Init(fullURL, METHOD);
}

HTTPS::~HTTPS()
{
	WSACleanup();

	if (ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
		ssl = nullptr;
	}

	if (ctx)
	{
		SSL_CTX_free(ctx);
		ctx = nullptr;
	}
}

bool HTTPS::Init(std::string& fullURL, const NET_SSL_METHOD METHOD)
{
	/* Initialize SSL */
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	// create ctx
	if ((ctx = SSL_CTX_new(NET_CREATE_SSL_OBJECT(METHOD))) == nullptr)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failed on creating SSL_CTX Object!"));
		return false;
	}

	LOG_DEBUG(CSTRING("[HTTPS] - using %s Methode!"), NET_GET_SSL_METHOD_NAME(METHOD));

	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

	const auto protoclPos = fullURL.find(CSTRING("://"));
	if (protoclPos == std::string::npos)
	{
		LOG_ERROR(CSTRING("[HTTPS] - URL is invalid! (missing Protocol)"));
		return false;
	}
	protocol = fullURL.substr(0, protoclPos);
	/*if (protocol != CSTRING("https"))
	{
		LOG_ERROR(CSTRING("[HTTPS] - Invalid Protocol, please use NET_HTTP"));
		return false;
	}*/

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

	WSADATA wsaData;

	const auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iResult != 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - WSAStartup failed with error: %d"), iResult);
		return false;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		LOG_ERROR(CSTRING("[HTTPS] - socket failed with error: %ld"), WSAGetLastError());
		WSACleanup();
		return false;
	}

	const auto host = gethostbyname(url.data());
	if (!host)
	{
		LOG_ERROR(CSTRING("[HTTPS] - Could not look up host: %s://%s%s:%i"), protocol.data(), url.data(), path.data(), port);
		return false;
	}

	// set SocketAddr
	auto addr = in_addr();
	memcpy(&addr, host->h_addr_list[0], sizeof(in_addr));
	memset(&connectSocketAddr, 0, sizeof(connectSocketAddr));
	connectSocketAddr.sin_family = AF_INET;
	connectSocketAddr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	connectSocketAddr.sin_port = htons(port);

	return true;
}

bool HTTPS::IsInited() const
{
	return Inited;
}

size_t HTTPS::DoSend(std::string& buffer) const
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
			{
				/* Some servers did not close the connection properly */
				//LOG_PEER(CSTRING("[HTTPS] - A non-recoverable, fatal error in the SSL library occurred, usually a protocol error. The OpenSSL error queue contains more information on the error. If this error occurs then no further I/O operations should be performed on the connection and SSL_shutdown() must not be called"));
				//return 0;
				break;
			}

			if (err == SSL_ERROR_WANT_WRITE)
				continue;

			LOG_DEBUG(CSTRING("[HTTPS] - Something bad happen... on Send"));
			return 0;
		}
	} while (res <= 0);

	return buffer.length();
}

size_t HTTPS::DoReceive(byte*& buffer) const
{
	size_t data_size = 0;
	for (;;)
	{
		auto tmpReceive = ALLOC<byte>(GetBufferSize() + 1);
		const auto result = static_cast<size_t>(SSL_read(ssl, tmpReceive, GetBufferSize()));
		if (result <= 0)
		{
			const auto err = SSL_get_error(ssl, result);
			if (err == SSL_ERROR_ZERO_RETURN)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTPS] - The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"));
				break;
			}
			if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTPS] - The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"));
				return 0;
			}
			if (err == SSL_ERROR_WANT_X509_LOOKUP)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTPS] - The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"));
				return 0;
			}
			if (err == SSL_ERROR_SYSCALL)
			{
				FREE(tmpReceive);
				LOG_PEER(CSTRING("[HTTPS] - Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"));
				return 0;
			}
			if (err == SSL_ERROR_SSL)
			{
				/* Some servers did not close the connection properly */
				FREE(tmpReceive);
				//LOG_PEER(CSTRING("[HTTPS] - A non-recoverable, fatal error in the SSL library occurred, usually a protocol error. The OpenSSL error queue contains more information on the error. If this error occurs then no further I/O operations should be performed on the connection and SSL_shutdown() must not be called"));
				//return 0;
				break;
			}
			if (err == SSL_ERROR_WANT_READ)
			{
				FREE(tmpReceive);
				continue;
			}

			FREE(tmpReceive);
			LOG_PEER(CSTRING("[HTTPS] - Something bad happen... on Receive"));
			return 0;
		}
		ERR_clear_error();
		tmpReceive[result] = '\0';

		if (!buffer)
		{
			buffer = ALLOC<byte>(result + 1);
			memcpy_s(buffer, result, tmpReceive, result);
			buffer[result] = '\0';
			data_size = result;
		}
		else
		{
			auto store = ALLOC<byte>(data_size + 1);
			memcpy_s(store, data_size, buffer, data_size);
			store[data_size] = '\0';
			FREE(buffer);
			buffer = ALLOC<byte>(data_size + result + 1);
			memcpy_s(buffer, (data_size + result), store, data_size);
			memcpy_s(&buffer[data_size], (data_size + result), tmpReceive, result);
			buffer[(data_size + result)] = '\0';
			FREE(store);
			data_size += result;
		}

		FREE(tmpReceive);
	}

	return data_size;
}

bool HTTPS::Get()
{
	if (!IsInited())
		return false;

	/* Connect to the server */
	if (connect(connectSocket, (sockaddr*)&connectSocketAddr, sizeof(connectSocketAddr)) == -1)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	/* Create a SSL object */
	if ((ssl = SSL_new(ctx)) == nullptr)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on creating ssl object"));
		return false;
	}

	/* Attach SSL to the socket */
	SSL_set_fd(ssl, static_cast<int>(connectSocket));

	/* Connect to SSL on the server side */
	if (SSL_connect(ssl) <= 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting ssl object to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	/* Set socket to non-blocking */
	unsigned long mode = 1;
	ioctlsocket(GetSocket(), FIONBIO, &mode);

	// Get Parameters
	auto params = GetParameters();

	// build request
	std::string req;
	req.append(CSTRING("GET "));
	req.append(GetPath());
	req.append(CSTRING(" HTTP/1.1"));
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Host: "));
	req.append(GetURL());
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));

	// Params
	req.append(params);
	params.clear();

	// Send Get Request
	if (DoSend(req))
	{
		ShutdownSocket();

		auto ret = 0;
		do
		{
			ret = SSL_shutdown(ssl);
			if (ret < 0)
				break;
		} while (ret == 0);

		// Receive Response
		byte* buffer = nullptr;
		if (DoReceive(buffer) != 0)
		{
			// Parse the result
			const auto res = ParseResult(buffer);
			FREE(buffer);
			return res;
		}

		FREE(buffer);
		return false;
	}

	return false;
}

bool HTTPS::Post()
{
	if (!IsInited())
		return false;

	/* Connect to the server */
	if (connect(connectSocket, (sockaddr*)&connectSocketAddr, sizeof(connectSocketAddr)) == -1)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
		return false;
	}

	/* Create a SSL object */
	if ((ssl = SSL_new(ctx)) == nullptr)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on creating ssl object"));
		return false;
	}

	/* Attach SSL to the socket */
	SSL_set_fd(ssl, static_cast<int>(connectSocket));

	/* Connect to SSL on the server side */
	if (SSL_connect(ssl) <= 0)
	{
		LOG_ERROR(CSTRING("[HTTPS] - failure on connecting ssl object to host: %s://%s%s:%i"), GetProtocol().data(), GetURL().data(), GetPath().data(), GetPort());
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
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Content-Length: "));
	std::stringstream param_length;
	param_length << params.length();
	req.append(param_length.str());
	param_length.clear();
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Content-Type: "));
	req.append(contentType.data());
	req.append(CSTRING("\r\n"));
	req.append(CSTRING("Connection: close"));
	req.append(CSTRING("\r\n\r\n"));
	
	// Params
	req.append(params);
	params.clear();

	// Send Post Request
	if (DoSend(req))
	{
		ShutdownSocket();

		auto ret = 0;
		do
		{
			ret = SSL_shutdown(ssl);
			if (ret < 0)
				break;
		} while (ret == 0);

		// Receive Response
		byte* buffer = nullptr;
		if (DoReceive(buffer) != 0)
		{
			// Parse the result
			const auto res = ParseResult(buffer);
			FREE(buffer);
			return res;
		}

		FREE(buffer);
		return false;
	}

	return false;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
