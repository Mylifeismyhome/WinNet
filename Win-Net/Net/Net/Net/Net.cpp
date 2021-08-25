#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/Net/NetCodes.h>

// Imports
#include <Net/Import/Kernel32.h>
#include <Net/Import/Ntdll.h>
#include <Net/Import/Ws2_32.h>

void Net::load()
{
#ifndef NET_DISABLE_IMPORT_KERNEL32
	Kernel32::Initialize();
#endif

#ifndef NET_DISABLE_IMPORT_NTDLL
	Ntdll::Initialize();
#endif

#ifndef NET_DISABLE_IMPORT_WS2_32
	Ws2_32::Initialize();
#endif

	Net::Codes::NetLoadErrorCodes();
}

void Net::unload()
{
#ifndef NET_DISABLE_IMPORT_WS2_32
	Ws2_32::Uninitialize();
#endif

#ifndef NET_DISABLE_IMPORT_NTDLL
	Ntdll::Uninitialize();
#endif

#ifndef NET_DISABLE_IMPORT_KERNEL32
	Kernel32::Uninitialize();
#endif

	Net::Codes::NetUnloadErrorCodes();
}

int Net::SocketOpt(SOCKET s, int level, int optname, SOCKET_OPT_TYPE optval, SOCKET_OPT_LEN optlen)
{
	if (s == INVALID_SOCKET) return -1;

#ifndef NET_DISABLE_IMPORT_WS2_32
	if(!Ws2_32::IsInitialized()) return setsockopt(s, level, optname, optval, optlen);
	return Ws2_32::setsockopt(s, level, optname, optval, optlen);
#else
	return setsockopt(s, level, optname, optval, optlen);
#endif
}

int Net::SetSocketOption(SOCKET socket, SocketOption_t<SOCKET_OPT_TYPE> opt)
{
	return Net::SocketOpt(socket,
		opt.level,
		opt.opt,
		opt.type,
		opt.len);
}

std::string Net::sock_err::getString(const int err, const bool is_ssl)
{
	if (is_ssl)
	{
		if (err == SSL_ERROR_ZERO_RETURN)
			return std::string(CSTRING("The TLS/SSL peer has closed the connection for writing by sending the close_notify alert. No more data can be read. Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the underlying transport has been closed"));
		else if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
			return std::string(CSTRING("The operation did not complete; the same TLS/SSL I/O function should be called again later. The underlying BIO was not connected yet to the peer and the call would block in connect()/accept(). The SSL function should be called again when the connection is established. These messages can only appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively. In order to find out, when the connection has been successfully established, on many platforms select() or poll() for writing on the socket file descriptor can be used"));
		else if (err == SSL_ERROR_WANT_X509_LOOKUP)
			return std::string(CSTRING("The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again. The TLS/SSL I/O function should be called again later. Details depend on the application"));
		else if (err == SSL_ERROR_SYSCALL)
			return std::string(CSTRING("Some non - recoverable, fatal I / O error occurred.The OpenSSL error queue may contain more information on the error.For socket I / O on Unix systems, consult errno for details.If this error occurs then no further I / O operations should be performed on the connection and SSL_shutdown() must not be called.This value can also be returned for other errors, check the error queue for details"));
	}
	else
	{
#ifdef BUILD_LINUX
		if (err == EACCES)
			return std::string(CSTRING("EACCES"));
		else if (err == EALREADY)
			return std::string(CSTRING("EALREADY"));
		else if (err == EBADF)
			return std::string(CSTRING("EBADF"));
		else if (err == ECONNRESET)
			return std::string(CSTRING("ECONNRESET"));
		else if (err == EDESTADDRREQ)
			return std::string(CSTRING("EDESTADDRREQ"));
		else if (err == EINTR)
			return std::string(CSTRING("EINTR"));
		else if (err == EINVAL)
			return std::string(CSTRING("EINVAL"));
		else if (err == EISCONN)
			return std::string(CSTRING("EISCONN"));
		else if (err == EMSGSIZE)
			return std::string(CSTRING("EMSGSIZE"));
		else if (err == ENOBUFS)
			return std::string(CSTRING("ENOBUFS"));
		else if (err == ENOMEM)
			return std::string(CSTRING("ENOMEM"));
		else if (err == ENOTCONN)
			return std::string(CSTRING("ENOTCONN"));
		else if (err == ENOTSOCK)
			return std::string(CSTRING("ENOTSOCK"));
		else if (err == EOPNOTSUPP)
			return std::string(CSTRING("EOPNOTSUPP"));
		else if (err == EPIPE)
			return std::string(CSTRING("EPIPE"));
		else if (err == EACCES)
			return std::string(CSTRING("EACCES"));
		else if (err == EACCES)
			return std::string(CSTRING("EACCES"));
		else if (err == EACCES)
			return std::string(CSTRING("EACCES"));
#else
		if (err == WSANOTINITIALISED)
			return std::string(CSTRING("A successful WSAStartup() call must occur before using this function"));
		else if (err == WSAENETDOWN)
			return std::string(CSTRING("The network subsystem has failed"));
		else if (err == WSAEFAULT)
			return std::string(CSTRING("The buf parameter is not completely contained in a valid part of the user address space"));
		else if (err == WSAENOTCONN)
			return std::string(CSTRING("The socket is not connected"));
		else if (err == WSAEINTR)
			return std::string(CSTRING("The (blocking) call was canceled through WSACancelBlockingCall()"));
		else if (err == WSAEINPROGRESS)
			return std::string(CSTRING("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback functione"));
		else if (err == WSAENETRESET)
			return std::string(CSTRING("The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress"));
		else if (err == WSAENOTSOCK)
			return std::string(CSTRING("The descriptor is not a socket"));
		else if (err == WSAEOPNOTSUPP)
			return std::string(CSTRING("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations"));
		else if (err == WSAESHUTDOWN)
			return std::string(CSTRING("The socket has been shut down; it is not possible to receive on a socket after shutdown() has been invoked with how set to SD_RECEIVE or SD_BOTH"));
		else if (err == WSAEMSGSIZE)
			return std::string(CSTRING("The message was too large to fit into the specified buffer and was truncated"));
		else if (err == WSAEINVAL)
			return std::string(CSTRING("The socket has not been bound with bind(), or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative"));
		else if (err == WSAECONNABORTED)
			return std::string(CSTRING("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable"));
		else if (err == WSAETIMEDOUT)
			return std::string(CSTRING("The connection has been dropped because of a network failure or because the peer system failed to respond"));
		else if (err == WSAECONNRESET)
			return std::string(CSTRING("The virtual circuit was reset by the remote side executing a hard or abortive close.The application should close the socket as it is no longer usable.On a UDP - datagram socket this error would indicate that a previous send operation resulted in an ICMP Port Unreachable message"));
		else if (err == WSAEHOSTUNREACH)
			return std::string(CSTRING("The remote host cannot be reached from this host at this time"));
#endif
	}

	return std::string(Net::String(CSTRING("The socket triggert an unknown error {code: %i, ssl_error: %s}"), err, is_ssl ? CSTRING("true") : CSTRING("false")).data().data());
}

std::string Net::ssl::GET_SSL_METHOD_NAME(const int method)
{
	for (auto& val : NET_SSL_METHOD_L)
	{
		if (val.method == method)
			return val.Name();
	}

	return std::string(CSTRING("UNKNOWN"));
}

std::string Net::ssl::NET_SSL_METHOD_T::Name() const
{
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLS)
		return std::string(CASTRING("TLS"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLS_SERVER)
		return std::string(CASTRING("TLS Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLS_CLIENT)
		return std::string(CASTRING("TLS Client"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv23)
		return std::string(CASTRING("SSLv23"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_SERVER)
		return std::string(CASTRING("SSLv23 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_CLIENT)
		return std::string(CASTRING("SSLv23 Client"));
#ifndef OPENSSL_NO_SSL3_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv3)
		return std::string(CASTRING("SSLv3"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_SERVER)
		return std::string(CASTRING("SSLv3 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_CLIENT)
		return std::string(CASTRING("SSLv3 Client"));
#endif
#ifndef OPENSSL_NO_TLS1_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1)
		return std::string(CASTRING("TLSv1"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_SERVER)
		return std::string(CASTRING("TLSv1 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_CLIENT)
		return std::string(CASTRING("TLSv1 Client"));
#endif
#ifndef OPENSSL_NO_TLS1_1_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1)
		return std::string(CASTRING("TLSv1_1"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_SERVER)
		return std::string(CASTRING("TLSv1_1 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_CLIENT)
		return std::string(CASTRING("TLSv1_1 Client"));
#endif
#ifndef OPENSSL_NO_TLS1_2_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2)
		return std::string(CASTRING("TLSv1_2"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_SERVER)
		return std::string(CASTRING("TLSv1_2 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_CLIENT)
		return std::string(CASTRING("TLSv1_2 Client"));
#endif
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLS)
		return std::string(CASTRING("DTLS"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLS_SERVER)
		return std::string(CASTRING("DTLS Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLS_CLIENT)
		return std::string(CASTRING("DTLS Client"));
#ifndef OPENSSL_NO_DTLS1_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1)
		return std::string(CASTRING("DTLSv1"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_SERVER)
		return std::string(CASTRING("DTLSv1 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_CLIENT)
		return std::string(CASTRING("DTLSv1 Client"));
#endif
#ifndef OPENSSL_NO_DTLS1_2_METHOD
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2)
		return std::string(CASTRING("DTLSv1_2"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_SERVER)
		return std::string(CASTRING("DTLSv1_2 Server"));
	if (method == NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_CLIENT)
		return std::string(CASTRING("DTLSv1_2 Client"));
#endif

	return std::string(CASTRING("UNKNOWN"));
}
