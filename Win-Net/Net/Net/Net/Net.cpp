#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

// Imports
#include <Net/Import/Kernel32.h>
#include <Net/Import/Ntdll.h>

void Net::load()
{
	Kernel32::Initialize();
	Ntdll::Initialize();
}

void Net::unload()
{
	Ntdll::Uninitialize();
	Kernel32::Uninitialize();
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