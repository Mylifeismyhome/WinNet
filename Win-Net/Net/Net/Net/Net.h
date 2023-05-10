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
#include <fcntl.h>
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
#ifdef BUILD_LINUX
#define NET_EXPORT_FUNCTION
#define NET_EXPORT_CLASS
#else
#ifdef DLL
#define NET_EXPORT_FUNCTION extern "C" __declspec(dllexport)
#define NET_EXPORT_CLASS __declspec(dllexport)
#else
#define NET_EXPORT_FUNCTION
#define NET_EXPORT_CLASS
#endif
#endif

#define NET_CALLBACK(type, name, ...) \
type name(__VA_ARGS__) override;

#define NET_DEFINE_CALLBACK(type, name, ...) \
virtual type name(__VA_ARGS__)

#define DEBUG_BREAK __debugbreak();

#define NET_CLASS(NAME, CLASS) class NAME final : public CLASS

#define NET_INITIALIZE(flag) Net::load(flag)
#define NET_LOAD(flag) Net::load(flag)
#define NET_UNLOAD Net::unload()

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

#define ALLOC NET_ALLOC_MEM
#define FREE NET_FREE_MEM

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
#endif

template <typename T, typename... Args>
T* NET_ALLOC_MEM(const size_t n = 1, Args... args)
{
	try
	{
		T* pointer = (T*)calloc(n, sizeof(T));
		if (pointer)
		{
#ifdef NET_TEST_MEMORY_LEAKS
			//printf("Allocated: %llu Byte(s) ; %p\n", n, pointer);
			NET_TEST_MEMORY_LEAKS_POINTER_LIST.emplace_back(pointer);
#endif

			/*
			* call constructor using placement new
			*/
			new (pointer) T(args...);

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

template <typename T>
__forceinline void NET_FREE_MEM(void* pointer)
{
	if (!pointer)
		return;

#ifdef NET_TEST_MEMORY_LEAKS
	//printf("Deallocated: %p\n", pointer);
	for (auto it = NET_TEST_MEMORY_LEAKS_POINTER_LIST.begin(); it != NET_TEST_MEMORY_LEAKS_POINTER_LIST.end(); ++it)
	{
		if (*it == pointer)
		{
			NET_TEST_MEMORY_LEAKS_POINTER_LIST.erase(it);
			break;
		}
	}
#endif

	/*
	* call destructor
	*/
	((T*)pointer)->~T();

	free(pointer);
	pointer = nullptr;
}

////////////////////////////////////////////////////
//    SECTION - PACKET Prefix & Suffix     //
//////////////////////////////////////////////////
#define NET_PACKET_BRACKET_OPEN CSTRING("{")
#define NET_PACKET_BRACKET_CLOSE CSTRING("}")

#define NET_RAW_DATA_KEY CSTRING("{RDK}")
#define NET_RAW_DATA_KEY_LEN 5
#define NET_RAW_DATA_ORIGINAL_SIZE CSTRING("{RDOS}")
#define NET_RAW_DATA_ORIGINAL_SIZE_LEN 6
#define NET_RAW_DATA CSTRING("{RD}")
#define NET_RAW_DATA_LEN 4

#define NET_DATA CSTRING("{D}")
#define NET_DATA_LEN 3

#define NET_PACKET_HEADER CSTRING("{BP}")
#define NET_PACKET_HEADER_LEN 4
#define NET_PACKET_FOOTER CSTRING("{EP}")
#define NET_PACKET_FOOTER_LEN 4

#define NET_PACKET_SIZE CSTRING("{PS}")
#define NET_PACKET_SIZE_LEN 4

#define NET_PACKET_ORIGINAL_SIZE CSTRING("{POS}")
#define NET_PACKET_ORIGINAL_SIZE_LEN 5

// Key is crypted using RSA
#define NET_AES_KEY CSTRING("{AK}")
#define NET_AES_KEY_LEN 4

// IV is crypted using RSA
#define NET_AES_IV CSTRING("{AV}")
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
		NET_LOG_ERROR(CSTRING("[%s] - Peer has no instance!"), SERVERNAME(this)); \
		stuff \
	}

#define PEER_NOT_VALID_EX(peer, server, stuff) if(!peer) \
	{ \
		NET_LOG_ERROR(CSTRING("[%s] - Peer has no instance!"), SERVERNAME(server)); \
		stuff \
	}

class SocketOptionInterface_t
{
public:
	SocketOptionInterface_t(int level, int opt)
	{
		this->level = level;
		this->opt = opt;
	}

	int level;
	int opt;
	virtual char* value() = 0;
	virtual int optlen() = 0;
};

template <class T>
class SocketOption_t : public SocketOptionInterface_t
{
	T _value;

public:
	SocketOption_t(int level, int opt, T value) : SocketOptionInterface_t(level, opt)
	{
		this->_value = value;
	}

	T val()
	{
		return _value;
	}

	char* value() override
	{
		return (char*)&_value;
	}

	void set(T val)
	{
		_value = val;
	}

	int optlen() override
	{
		return sizeof(T);
	}
};

class OptionInterface_t
{
public:
	OptionInterface_t(DWORD opt)
	{
		this->opt = opt;
	}

	DWORD opt;

	virtual int optlen() = 0;
};

template <class T>
class Option_t : public OptionInterface_t
{
	T _value;

public:
	Option_t(DWORD opt, T value) : OptionInterface_t(opt)
	{
		this->_value = value;
	}

	T value()
	{
		return _value;
	}

	void set(T val)
	{
		_value = val;
	}

	int optlen() override
	{
		return sizeof(T);
	}
};

/////////////////////////////////////////////////////
//    SECTION - SSL Methode Definitions     //
////////////////////////////////////////////////////
namespace Net
{
	enum NetOptions
	{
		NONE = 0,
		ENABLE_LOGGING
	};

	NET_EXPORT_FUNCTION void load(int flag = NetOptions::NONE);
	NET_EXPORT_FUNCTION void unload();

	int SocketOpt(SOCKET, int, int, SOCKET_OPT_TYPE, SOCKET_OPT_LEN);

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

	////////////////////////////////////
	//    USEFULL FUNCTIONS    //
	///////////////////////////////////
	bool NET_STRING_IS_NUMBER(const std::string& s);
}

namespace ServerHandshake
{
	enum Server_HandshakeRet_t
	{
		is_not_websocket = 0,
		is_websocket,
		error,
		peer_not_valid,
		would_block
	};
}

namespace WebServerHandshake
{
	enum HandshakeRet_t
	{
		success = 0,
		missmatch,
		error,
		peer_not_valid,
		would_block
	};
}

/* OPTION BIT FLAGS */
#define NET_OPT_FREQUENZ (1 << 0)
#define NET_OPT_MODE_BLOCKING (1 << 1)
#define NET_OPT_USE_CIPHER (1 << 2)
#define NET_OPT_CIPHER_RSA_SIZE (1 << 3)
#define NET_OPT_CIPHER_AES_SIZE (1 << 4)
#define NET_OPT_USE_COMPRESSION (1 << 5)
#define NET_OPT_INTERVAL_LATENCY (1 << 6)
#define NET_OPT_NAME (1 << 7)
#define NET_OPT_PORT (1 << 8)
#define NET_OPT_SSL (1 << 9)
#define NET_OPT_SSL_METHOD (1 << 10)
#define NET_OPT_SSL_CERT (1 << 11)
#define NET_OPT_SSL_KEY (1 << 12)
#define NET_OPT_SSL_CA (1 << 13)
#define NET_OPT_WS_CUSTOM_HANDSHAKE (1 << 14)
#define NET_OPT_WS_CUSTOM_ORIGIN (1 << 15)
#define NET_OPT_WS_NO_HANDSHAKE (1 << 16)
#define NET_OPT_DISABLE_LATENCY_REQUEST (1 << 17)

/* TIMER TO DISCONNECT PEERS FROM USING WRONG PROTOCOL */
#define NET_OPT_NET_PROTOCOL_CHECK_TIME (1 << 18)

#define NET_OPT_NET_PROTOCOL_HEARTBEAT_INTERVAL (1 << 19)
#define NET_OPT_NET_PROTOCOL_HEARTBEAT_TOLERANT_TIME (1 << 20)

#define NET_OPT_MAX_PEERS_THREAD (1 << 21)

#define NET_OPT_USE_HEARTBEAT (1 << 22)

/* Server & Client Option */

/*
* this option will force the library to spawn a thread to execute the packet in there
* without this option, any callback that might perform some big job will block the socket from further execution
* enable this option to perform the job in a seperate thread
*/
#define NET_OPT_EXECUTE_PACKET_ASYNC (1 << 23)
#define NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC false

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

/* TIMER TO DISCONNECT PEERS FROM USING WRONG PROTOCOL */
#define NET_OPT_DEFAULT_NET_PROTOCOL_CHECK_TIME 60000

#define NET_OPT_DEFAULT_NET_PROTOCOL_HEARTBEAT_INTERVAL 10000
#define NET_OPT_DEFAULT_NET_PROTOCOL_HEARTBEAT_TOLERANT_TIME 20000

#define NET_OPT_DEFAULT_MAX_PEERS_THREAD 4

#define NET_OPT_DEFAULT_USE_HEARTBEAT true

#define NOEXCEPT noexcept
#define CONSTEXPR const

#ifdef BUILD_LINUX
#define FUNCNAME __FUNCTION__
#else
#define FUNCNAME __FUNCTION__
#endif

#ifdef BUILD_LINUX
#define __int64 long long
#endif