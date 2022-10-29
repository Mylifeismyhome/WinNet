#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/Net/NetCodes.h>
#include <Net/Import/ImportResolver.h>
#include <Net/Import/Ws2_32.hpp>

extern void __Net_Enable_Logging();
extern void __Net_Shutdown_Logging();

#ifdef NET_TEST_MEMORY_LEAKS
static void NET_TEST_MEMORY_SHOW_DIAGNOSTIC()
{
	printf("----- POINTER INSTANCE(s) -----\n");
	for (const auto entry : NET_TEST_MEMORY_LEAKS_POINTER_LIST)
		printf("Allocated Instance: %p\n", entry);
	printf("----------------------------------------\n");
}
#endif

NET_EXPORT_FUNCTION void Net::load(int flag)
{
#ifndef NET_DISABLE_IMPORT_KERNEL32
	Import::Resolver::Load(CSTRING("Kernel32"), CSTRING(WINDOWS_MODULE_PATH"\\kernel32.dll"), Import::Resolver::type_t::RESOLVE_MEMORY);
#endif

#ifndef NET_DISABLE_IMPORT_NTDLL
	Import::Resolver::Load(CSTRING("Ntdll"), CSTRING(WINDOWS_MODULE_PATH"\\ntdll.dll"));
#endif

#ifndef NET_DISABLE_IMPORT_WS2_32
	Import::Resolver::Load(CSTRING("Ws2_32"), CSTRING(WINDOWS_MODULE_PATH"\\Ws2_32.dll"));
#endif

#ifndef NET_DISABLE_LOGMANAGER
	if (flag & NetOptions::ENABLE_LOGGING)
	{
		__Net_Enable_Logging();
	}
#endif

	Net::Codes::NetLoadErrorCodes();
}

NET_EXPORT_FUNCTION void Net::unload()
{
#ifndef NET_DISABLE_LOGMANAGER
	__Net_Shutdown_Logging();
#endif

	Net::Codes::NetUnloadErrorCodes();

#ifndef NET_DISABLE_IMPORT_WS2_32
	Import::Resolver::Unload(CSTRING("Ws2_32"));
#endif

#ifndef NET_DISABLE_IMPORT_NTDLL
	Import::Resolver::Unload(CSTRING("Ntdll"));
#endif

#ifndef NET_DISABLE_IMPORT_KERNEL32
	Import::Resolver::Unload(CSTRING("Kernel32"));
#endif

#ifdef NET_TEST_MEMORY_LEAKS
	NET_TEST_MEMORY_SHOW_DIAGNOSTIC();
#endif
}

int Net::SocketOpt(SOCKET s, int level, int optname, SOCKET_OPT_TYPE optval, SOCKET_OPT_LEN optlen)
{
	if (s == INVALID_SOCKET) return -1;

#ifndef NET_DISABLE_IMPORT_WS2_32
	return Ws2_32::setsockopt(s, level, optname, optval, optlen);
#else
	return setsockopt(s, level, optname, optval, optlen);
#endif
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
			return std::string(CSTRING("WSANOTINITIALISED"));
		else if (err == WSAENETDOWN)
			return std::string(CSTRING("WSAENETDOWN"));
		else if (err == WSAEFAULT)
			return std::string(CSTRING("WSAEFAULT"));
		else if (err == WSAENOTCONN)
			return std::string(CSTRING("WSAENOTCONN"));
		else if (err == WSAEINTR)
			return std::string(CSTRING("WSAEINTR"));
		else if (err == WSAEINPROGRESS)
			return std::string(CSTRING("WSAEINPROGRESS"));
		else if (err == WSAENETRESET)
			return std::string(CSTRING("WSAENETRESET"));
		else if (err == WSAENOTSOCK)
			return std::string(CSTRING("WSAENOTSOCK"));
		else if (err == WSAEOPNOTSUPP)
			return std::string(CSTRING("WSAEOPNOTSUPP"));
		else if (err == WSAESHUTDOWN)
			return std::string(CSTRING("WSAESHUTDOWN"));
		else if (err == WSAEMSGSIZE)
			return std::string(CSTRING("WSAEMSGSIZE"));
		else if (err == WSAEINVAL)
			return std::string(CSTRING("WSAEINVAL"));
		else if (err == WSAECONNABORTED)
			return std::string(CSTRING("WSAECONNABORTED"));
		else if (err == WSAETIMEDOUT)
			return std::string(CSTRING("WSAETIMEDOUT"));
		else if (err == WSAECONNRESET)
			return std::string(CSTRING("WSAECONNRESET"));
		else if (err == WSAEHOSTUNREACH)
			return std::string(CSTRING("WSAEHOSTUNREACH"));
#endif
	}

	/*
	* must be const char*
	* need to create a copy of it
	*/
	return std::string(Net::String(reinterpret_cast<const char*>(CSTRING("unknown error {code: %i, ssl_error: %s}")), err, is_ssl ? CSTRING("true") : CSTRING("false")).data().data());
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

bool Net::NET_STRING_IS_NUMBER(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}