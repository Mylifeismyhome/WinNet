#pragma once
#include "NetBuild.h"

#ifdef BUILD_LINUX
#define NET_IGNORE_CONVERSION_NULL \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wconversion-null\"")

#define NET_POP \
    _Pragma("GCC diagnostic pop")
#else
#pragma warning(disable: 4996)
#pragma warning(disable: 4006)
#pragma warning(disable: 4081)
#pragma warning(disable: 4311)
#pragma warning(disable: 4530)
#pragma warning(disable: 4302)
#pragma warning(disable: 4996)
#pragma warning(disable: 4267)
#pragma warning(disable: 4005)
#define _WINSOCKET_DEPRECATED_NO_WARNINGS

#define NET_IGNORE_CONVERSION_NULL
#define NET_POP
#endif

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <thread>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include <memory>
#include <chrono>

#ifndef BUILD_LINUX
#include <intrin.h>
#endif

#include <cstdlib>
#include <cstring>
#include <sys/types.h>

#ifdef BUILD_LINUX
#include <linux/limits.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <cmath>
#endif

#ifndef BUILD_LINUX
#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <Windows.h>
#include <wincrypt.h>
#endif

#include <openssl/ssl.h>

#ifdef BUILD_LINUX
typedef unsigned long DWORD;
typedef unsigned char byte;
typedef unsigned char BYTE;
typedef int SOCKET;
typedef int64_t int64;
typedef unsigned int uint;
typedef uint64_t uint64;
#define __forceinline inline
#define MAX_PATH 260
#define closesocket close
#define ioctlsocket ioctl
#define SOCKET_RD SHUT_RD /* Disables further receive operations. */
#define SOCKET_WR SHUT_WR /* Disables further send operations. */
#define SOCKET_RDWR SHUT_RDWR /* Disables further send and receive operations. */
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKET_OPT_TYPE void*
#define SOCKET_OPT_LEN socklen_t
#define ERRNO_ERROR_TRIGGERED errno != -1 && errno != 0
#else
typedef __int64 int64;
typedef unsigned int uint;
typedef unsigned __int64 uint64;
#define SOCKET_RD SD_SEND
#define SOCKET_WR SD_SEND
#define SOCKET_RDWR SD_SEND
#define SOCKET_OPT_TYPE char*
#define SOCKET_OPT_LEN int
#define MSG_NOSIGNAL 0
#endif

///////////////////////////////////
//    SECTION - DEFINES     //
//////////////////////////////////
#ifdef DLL
#define NET_API extern "C" __declspec(dllexport)
#else
#define NET_API
#endif

#define NET_NAMESPACE_BEGIN(n) namespace n {
#define NET_NAMESPACE_END }

#define NET_CLASS_BEGIN(c) class NET_API c {
#define NET_ABSTRAC_CLASS_BEGIN(c, d) class NET_API c : public d {
#define NET_CLASS_END };
#define NET_CLASS_CONSTRUCTUR(a, ...) a(__VA_ARGS__);
#define NET_CLASS_CONSTRUCTUR_NOEXCEPT(a, ...) a(__VA_ARGS__) NOEXPECT;
#define NET_CLASS_VCONSTRUCTUR(a, ...) virtual a(__VA_ARGS__);
#define NET_CLASS_DESTRUCTUR(d) ~d();
#define NET_CLASS_VDESTRUCTUR(d) virtual ~d();
#define NET_CLASS_BEGIN_CONSTRUCTUR(a, ...) a(__VA_ARGS__) {
#define NET_CLASS_BEGIN_VCONSTRUCTUR(a, ...) virtual a(__VA_ARGS__) {
#define NET_CLASS_BEGIN_DESTRUCTUR(d) ~d() {
#define NET_CLASS_BEGIN_VDESTRUCTUR(d) virtual ~d() {
#define NET_CLASS_END_CONTRUCTION }
#define NET_CLASS_END_DESTRUCTUR }
#define NET_CLASS_PRIVATE private:
#define NET_CLASS_PUBLIC public:
#define NET_CLASS_PROTECTED protected:

#define NET_STRUCT_BEGIN(s) struct s {
#define NET_ABSTRACT_STRUCT(s, t) struct s : t {
#define NET_STRUCT_END };
#define NET_STRUCT_CONSTRUCTUR(c) c();
#define NET_STRUCT_VCONSTRUCTUR(c) virtual c();
#define NET_STRUCT_DESTRUCTUR(d) ~d();
#define NET_STRUCT_VDESTRUCTUR(d) virtual ~d();
#define NET_STRUCT_BEGIN_CONSTRUCTUR(c) c() {
#define NET_STRUCT_BEGIN_VCONSTRUCTUR(c) virtual c() {
#define NET_STRUCT_BEGIN_DESTRUCTUR(d) ~d() {
#define NET_STRUCT_BEGIN_VDESTRUCTUR(d) virtual ~d() {
#define NET_STRUCT_END_CONTRUCTION }

#define NET_CALLBACK(type, name, ...) \
type name(__VA_ARGS__) override;

#define NET_DEFINE_CALLBACK(type, name, ...) \
virtual type name(__VA_ARGS__)

#define DEBUG_BREAK __debugbreak();

#define NET_SERVER_BEGIN_DATA_PACKAGE_NATIVE(classname) \
bool classname::CheckDataN(NET_PEER peer, const int id, NET_PACKAGE pkg) \
{ \
if(!peer) \
	return false; \
switch (id) \
{

#define NET_SERVER_BEGIN_DATA_PACKAGE(classname) \
bool classname::CheckData(NET_PEER peer, const int id, NET_PACKAGE pkg) \
{ \
if(!peer || !peer->estabilished) \
	return false; \
switch (id) \
{

#define NET_SERVER_DEFINE_PACKAGE(xxx, yyy) \
    case yyy: \
    { \
      On##xxx(peer, pkg); \
      break; \
    }

#define NET_SERVER_END_DATA_PACKAGE \
	default: \
		return false; \
} \
return true; \
}

#define NET_CLIENT_BEGIN_DATA_PACKAGE_NATIVE(classname) \
bool classname::CheckDataN(const int id, NET_PACKAGE pkg) \
{ \
switch (id) \
{

#define NET_CLIENT_BEGIN_DATA_PACKAGE(classname) \
bool classname::CheckData(const int id, NET_PACKAGE pkg) \
{ \
switch (id) \
{

#define NET_CLIENT_DEFINE_PACKAGE(xxx, yyy) \
    case yyy: \
    { \
      On##xxx(pkg); \
      break; \
    } \

#define NET_CLIENT_END_DATA_PACKAGE \
	default: \
		return false; \
} \
return true; \
}

#define NET_INHERITANCE(NAME, CLASS) class NAME final : public CLASS
#define NET_CLIENT_CLASS(NAME) NET_INHERITANCE(NAME, NET_CLIENT)
#define NET_SERVER_CLASS(NAME) NET_INHERITANCE(NAME, NET_SERVER)
#define NET_WEBSERVER_CLASS(NAME) NET_INHERITANCE(NAME, NET_WEB_SERVER)

/* DATA STRUCTURE ALIGNEMNT */
#ifndef BUILD_LINUX
#define NET_DSA_BEGIN __pragma("pack(push)") \
 __pragma("pack(1)")
#define NET_DSA_END __pragma("pack(pop)")
#else
#define NET_DSA_BEGIN _Pragma("pack(push)") \
 _Pragma("pack(1)")
#define NET_DSA_END _Pragma("pack(pop)")
#endif

#define ALLOC Alloc
#define FREE(data) Free(data)

////////////////////////////////////////////////
//    SECTION - TYPE DEFENITIONS     //
///////////////////////////////////////////////
typedef unsigned int INDEX;

#ifdef _WIN64
typedef DWORD64 typeLatency;
#else
typedef DWORD typeLatency;
#endif

//////////////////////////////////////////////////////
//    SECTION - Allocation & Deallocation     //
////////////////////////////////////////////////////
#ifdef NET_TEST_MEMORY_LEAKS
static std::vector<void*> NET_TEST_MEMORY_LEAKS_POINTER_LIST;

__forceinline void NET_TEST_MEMORY_SHOW_DIAGNOSTIC()
{
	printf(CSTRING("----- POINTER INSTANCE(s) -----\n"));
	for (const auto entry : NET_TEST_MEMORY_LEAKS_POINTER_LIST)
		printf(CSTRING("Allocated Instance: %p\n"), entry);
	printf(CSTRING("----------------------------------------\n"));
}
#endif

template <typename T>
T* NET_ALLOC_MEM(const size_t n)
{
	try
	{
		T* pointer = new T[n];
		if (pointer)
		{
#ifdef NET_TEST_MEMORY_LEAKS
			printf(CSTRING("Allocated: %llu Byte(s) ; %p\n"), n, pointer);
			NET_TEST_MEMORY_LEAKS_POINTER_LIST.emplace_back(pointer);
#endif

			return pointer;
		}

		throw std::bad_alloc();
	}
	catch (...)
	{
		// failure on allocating
		return nullptr;
	}
}

__forceinline void NET_FREE_MEM(void* pointer)
{
	if (!pointer)
		return;

#ifdef NET_TEST_MEMORY_LEAKS
	printf(CSTRING("Deallocated: %p\n"), pointer);
	for (auto it = NET_TEST_MEMORY_LEAKS_POINTER_LIST.begin(); it != NET_TEST_MEMORY_LEAKS_POINTER_LIST.end(); ++it)
	{
		if (*it == pointer)
		{
			NET_TEST_MEMORY_LEAKS_POINTER_LIST.erase(it);
			break;
		}
	}
#endif

	free(pointer);
	pointer = nullptr;
}

template<class T>
T* Alloc(const size_t size = 1)
{
	T* object = NET_ALLOC_MEM<T>(size);
	return object;
}

template <typename T>
static void Free(T*& data)
{
	NET_FREE_MEM(data);
	data = nullptr;
}

////////////////////////////////////////////////////
//    SECTION - Package Prefix & Suffix     //
//////////////////////////////////////////////////
#define NET_PACKAGE_BRACKET_OPEN CSTRING("{")
#define NET_PACKAGE_BRACKET_CLOSE CSTRING("}")

#define NET_RAW_DATA_KEY CSTRING("{RAW DATA KEY}")
#define NET_RAW_DATA CSTRING("{RAW DATA}")

#define NET_DATA CSTRING("{DATA}")
#define NET_DATA_LEN 6

#define NET_PACKAGE_HEADER CSTRING("{BEGIN PACKAGE}")
#define NET_PACKAGE_HEADER_LEN 15
#define NET_PACKAGE_FOOTER CSTRING("{END PACKAGE}")
#define NET_PACKAGE_FOOTER_LEN 13

#define NET_PACKAGE_SIZE CSTRING("{PACKAGE SIZE}")
#define NET_PACKAGE_SIZE_LEN 14

// Key is crypted using RSA
#define NET_AES_KEY CSTRING("{KEY}")
#define NET_AES_KEY_LEN 5

// IV is crypted using RSA
#define NET_AES_IV CSTRING("{IV}")
#define NET_AES_IV_LEN 4

#define NET_UID size_t
#define INVALID_UID  (size_t)(~0)
#define INVALID_SIZE (size_t)(~0)
#define INVALID_UINT_SIZE (uint32_t)(~0)

#define SOCKET_VALID(socket) if(socket != INVALID_SOCKET)
#define SOCKET_NOT_VALID(socket) if(socket == INVALID_SOCKET)

#define PEER_VALID(peer) if(peer)
#define PEER_NOT_VALID(peer, stuff) if(!peer) \
	{ \
		LOG_ERROR(CSTRING("[%s] - Peer has no instance!"), SERVERNAME(this)); \
		stuff \
	}

#define PEER_NOT_VALID_EX(peer, server, stuff) if(!peer) \
	{ \
		LOG_ERROR(CSTRING("[%s] - Peer has no instance!"), SERVERNAME(server)); \
		stuff \
	}

template <class T>
NET_STRUCT_BEGIN(SocketOption_t)
int level;
int opt;
T type;
SOCKET_OPT_LEN len;

SocketOption_t()
{
        this->level = NULL;
        this->opt = NULL;
        this->len = INVALID_SIZE;
}

SocketOption_t(const int level, const int opt, const T type)
{
        this->level = opt;
        this->opt = opt;
        this->type = type;
        this->len = sizeof(type);
}
NET_STRUCT_END

template <class T>
NET_STRUCT_BEGIN(Option_t)
DWORD opt;
T type;
SOCKET_OPT_LEN len;

Option_t()
{
        this->opt = NULL;
        this->len = INVALID_SIZE;
}

Option_t(const DWORD opt, const T type)
{
        this->opt = opt;
        this->type = type;
        this->len = sizeof(type);
}
NET_STRUCT_END

/////////////////////////////////////////////////////
//    SECTION - SSL Methode Definitions     //
////////////////////////////////////////////////////
namespace Net
{
	void load();
	void unload();

	int SocketOpt(SOCKET, int, int, SOCKET_OPT_TYPE, SOCKET_OPT_LEN);
	int SetSocketOption(SOCKET, SocketOption_t<SOCKET_OPT_TYPE>);

	namespace sock_err
	{
		std::string getString(const int, bool = false);
	};

	namespace ssl
	{
		struct NET_SSL_METHOD_T
		{
			int method;

			std::string Name() const;
		};

		enum NET_SSL_METHOD
		{
			NET_SSL_METHOD_TLS,
			NET_SSL_METHOD_TLS_SERVER,
			NET_SSL_METHOD_TLS_CLIENT,
			NET_SSL_METHOD_SSLv23,
			NET_SSL_METHOD_SSLv23_SERVER,
			NET_SSL_METHOD_SSLv23_CLIENT,
#ifndef OPENSSL_NO_SSL3_METHOD
			NET_SSL_METHOD_SSLv3,
			NET_SSL_METHOD_SSLv3_SERVER,
			NET_SSL_METHOD_SSLv3_CLIENT,
#endif
#ifndef OPENSSL_NO_TLS1_METHOD
			NET_SSL_METHOD_TLSv1,
			NET_SSL_METHOD_TLSv1_SERVER,
			NET_SSL_METHOD_TLSv1_CLIENT,
#endif
#ifndef OPENSSL_NO_TLS1_1_METHOD
			NET_SSL_METHOD_TLSv1_1,
			NET_SSL_METHOD_TLSv1_1_SERVER,
			NET_SSL_METHOD_TLSv1_1_CLIENT,
#endif
#ifndef OPENSSL_NO_TLS1_2_METHOD
			NET_SSL_METHOD_TLSv1_2,
			NET_SSL_METHOD_TLSv1_2_SERVER,
			NET_SSL_METHOD_TLSv1_2_CLIENT,
#endif
			NET_SSL_METHOD_DTLS,
			NET_SSL_METHOD_DTLS_SERVER,
			NET_SSL_METHOD_DTLS_CLIENT,
#ifndef OPENSSL_NO_DTLS1_METHOD
			NET_SSL_METHOD_DTLSv1,
			NET_SSL_METHOD_DTLSv1_SERVER,
			NET_SSL_METHOD_DTLSv1_CLIENT,
#endif
#ifndef OPENSSL_NO_DTLS1_2_METHOD
			NET_SSL_METHOD_DTLSv1_2,
			NET_SSL_METHOD_DTLSv1_2_SERVER,
			NET_SSL_METHOD_DTLSv1_2_CLIENT,
#endif

			NET_LAST_NET_SSL_METHOD
		};

		static NET_SSL_METHOD_T NET_SSL_METHOD_L[] =
		{
			{NET_SSL_METHOD::NET_SSL_METHOD_TLS},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLS_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLS_CLIENT},
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23},
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_CLIENT},
		#ifndef OPENSSL_NO_SSL3_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3},
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_CLIENT},
		#endif
		#ifndef OPENSSL_NO_TLS1_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_CLIENT},
		#endif
		#ifndef OPENSSL_NO_TLS1_1_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_CLIENT},
		#endif
		#ifndef OPENSSL_NO_TLS1_2_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_CLIENT},
		#endif
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLS},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLS_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLS_CLIENT},
		#ifndef OPENSSL_NO_DTLS1_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_CLIENT},
		#endif
		#ifndef OPENSSL_NO_DTLS1_2_METHOD
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_SERVER},
			{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_CLIENT}
		#endif
		};

		inline const SSL_METHOD* NET_CREATE_SSL_OBJECT(const int method)
		{
			for (auto& val : NET_SSL_METHOD_L)
			{
				if (val.method == method)
				{
					switch (val.method)
					{
					case NET_SSL_METHOD::NET_SSL_METHOD_TLS:
						return TLS_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLS_SERVER:
						return TLS_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLS_CLIENT:
						return TLS_client_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv23:
						return SSLv23_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_SERVER:
						return SSLv23_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_CLIENT:
						return SSLv23_client_method();

#ifndef OPENSSL_NO_SSL3_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv3:
						return SSLv3_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_SERVER:
						return SSLv3_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_CLIENT:
						return SSLv3_client_method();
#endif

#ifdef BUILD_LINUX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifndef OPENSSL_NO_TLS1_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1:
						return TLSv1_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_SERVER:
						return TLSv1_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_CLIENT:
						return TLSv1_client_method();
#endif

#ifndef OPENSSL_NO_TLS1_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1:
						return TLSv1_1_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_SERVER:
						return TLSv1_1_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_CLIENT:
						return TLSv1_1_client_method();
#endif

#ifndef OPENSSL_NO_TLS1_2_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2:
						return TLSv1_2_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_SERVER:
						return TLSv1_2_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_CLIENT:
						return TLSv1_2_client_method();
#endif
#ifdef BUILD_LINUX
#pragma GCC diagnostic pop
#endif

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLS:
						return DTLS_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLS_SERVER:
						return DTLS_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLS_CLIENT:
						return DTLS_client_method();

#ifdef BUILD_LINUX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifndef OPENSSL_NO_DTLS1_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1:
						return DTLSv1_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_SERVER:
						return DTLSv1_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_CLIENT:
						return DTLSv1_client_method();
#endif

#ifndef OPENSSL_NO_DTLS1_2_METHOD
					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2:
						return DTLSv1_2_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_SERVER:
						return DTLSv1_2_server_method();

					case NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_CLIENT:
						return DTLSv1_2_client_method();
#endif
#ifdef BUILD_LINUX
#pragma GCC diagnostic pop
#endif
					default:
						break;
					}
				}
			}

			return nullptr;
		}

		std::string GET_SSL_METHOD_NAME(const int method);
	}
}

////////////////////////////////////
//    USEFULL FUNCTIONS    //
///////////////////////////////////
#ifdef VS13
static bool NET_STRING_IS_NUMBER(const std::string& s)
{
	for (size_t it = 0; it < s.length(); ++it)
		if (s[it] < '0' || s[it] > '9') return false;
	return true;
}
#else
static bool NET_STRING_IS_NUMBER(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}
#endif

NET_NAMESPACE_BEGIN(ServerHandshake)
enum Server_HandshakeRet_t
{
	is_not_websocket = 0,
	is_websocket,
	error,
	peer_not_valid,
	would_block
};
NET_NAMESPACE_END

NET_NAMESPACE_BEGIN(WebServerHandshake)
enum HandshakeRet_t
{
	success = 0,
	missmatch,
	error,
	peer_not_valid,
	would_block
};
NET_NAMESPACE_END

/* OPTION BIT FLAGS */
#define NET_OPT_FREQUENZ (1 << 0)
#define NET_OPT_MODE_BLOCKING (1 << 1)
#define NET_OPT_USE_CIPHER (1 << 2)
#define NET_OPT_CIPHER_RSA_SIZE (1 << 3)
#define NET_OPT_CIPHER_AES_SIZE (1 << 4)
#define NET_OPT_USE_COMPRESSION (1 << 5)
#define NET_OPT_INTERVAL_LATENCY (1 << 6)
#define NET_OPT_TIMEOUT_TCP_READ (1 << 7)
#define NET_OPT_NAME (1 << 8)
#define NET_OPT_PORT (1 << 9)
#define NET_OPT_SSL (1 << 10)
#define NET_OPT_SSL_METHOD (1 << 11)
#define NET_OPT_SSL_CERT (1 << 12)
#define NET_OPT_SSL_KEY (1 << 13)
#define NET_OPT_SSL_CA (1 << 14)
#define NET_OPT_WS_CUSTOM_HANDSHAKE (1 << 15)
#define NET_OPT_WS_CUSTOM_ORIGIN (1 << 16)
#define NET_OPT_WS_NO_HANDSHAKE (1 << 17)
#define NET_OPT_DISABLE_LATENCY_REQUEST (1 << 18)

/* RELEVANT TO NTP PROTOCOL (ANTI-PACKAGE EMULATION) */
#define NET_OPT_USE_NTP (1 << 19)
#define NET_OPT_NTP_HOST (1 << 20)
#define NET_OPT_NTP_PORT (1 << 21)
#define NET_OPT_USE_TOTP (1 << 22)
#define NET_OPT_TOTP_INTERVAL (1 << 23)
#define NET_OPT_NTP_SYNC_INTERVAL (1 << 24)

/* DEFAULT OPTION VALUES */
#define NET_OPT_DEFAULT_MAX_PACKET_SIZE 65535
#define NET_OPT_DEFAULT_RSA_SIZE 1024
#define NET_OPT_DEFAULT_AES_SIZE CryptoPP::AES::MAX_KEYLENGTH
#define NET_OPT_DEFAULT_USE_CIPHER false
#define NET_OPT_DEFAULT_USE_COMPRESSION false
#define NET_OPT_DEFAULT_FREQUENZ 66
#define NET_OPT_DEFAULT_MODE_BLOCKING false
#define NET_OPT_DEFAULT_INTERVAL_LATENCY 1000
#define NET_OPT_DEFAULT_TIMEOUT_TCP_READ 10
#define NET_OPT_DEFAULT_NAME CSTRING("UNKNOWN")
#define NET_OPT_DEFAULT_PORT 1337
#define NET_OPT_DEFAULT_LATENCY_REQUEST false

/* WEBSERVER RELATED */
#define NET_OPT_DEFAULT_SSL false
#define NET_OPT_DEFAULT_SSL_METHOD Net::ssl::NET_SSL_METHOD::NET_SSL_METHOD_TLS
#define NET_OPT_DEFAULT_SSL_CERT CSTRING("cert.pem")
#define NET_OPT_DEFAULT_SSL_KEY CSTRING("key.pem")
#define NET_OPT_DEFAULT_SSL_CA CSTRING("servercert.pem")
#define NET_OPT_DEFAULT_WS_CUSTOM_HANDSHAKE false
#define NET_OPT_DEFAULT_WS_CUSTOM_ORIGIN CSTRING("localhost")
#define NET_OPT_DEFAULT_WS_NO_HANDSHAKE false

/* RELEVANT TO NTP PROTOCOL (ANTI-PACKAGE EMULATION) */
#define NET_OPT_DEFAULT_USE_NTP false
#define NET_OPT_DEFAULT_NTP_HOST Net::Protocol::NTP::ResolveHostname(CSTRING("time.google.com"))
#define NET_OPT_DEFAULT_NTP_PORT 123
#define NET_OPT_DEFAULT_USE_TOTP false
#define NET_OPT_DEFAULT_TOTP_INTERVAL 60
#define NET_OPT_DEFAULT_NTP_SYNC_INTERVAL 30000

#ifndef VS13
#define NOEXPECT noexcept
#define CONSTEXPR constexpr
#else
#define NOEXPECT
#define CONSTEXPR const
#endif

#define FUNCNAME CSTRING(__FUNCTION__)

#ifdef BUILD_LINUX
#define __int64 long long
#endif