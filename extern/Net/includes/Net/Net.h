#pragma once
#pragma warning(disable: 4996)
#pragma warning(disable: 4006)

#include <stdio.h>
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
#include <intrin.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <windows.h>
#include <wincrypt.h>

#include <OpenSSL/include/openssl/ssl.h>

/////////////////////////////////////////////////////
//    SECTION - Library Version & Version Key     //
///////////////////////////////////////////////////
/* Net Key to compare on which version we are running */
#define NetMajorVersion 3 // Re-Code - Library Changes
#define NetMinorVersion 4 // Function extension
#define NetRevision 4 // Issue fixing
#define NetKey "1MFOm3a9as-xieg1iEMIf-pgKHPNlSMP-pgKHPNlSMP"

///////////////////////////////
//    SECTION - DEFINES     //
/////////////////////////////
#ifdef DLL
#define NET_API __declspec(dllexport)
#else
#define NET_API
#endif

#define BEGIN_NAMESPACE(n) namespace n {
#define END_NAMESPACE }

#define BEGIN_CLASS(c) class NET_API c {
#define BEGIN_CLASS2(c, d) class NET_API c : public d {
#define BEGIN_CLASS3(c, d, e) class NET_API c : public d, e {
#define BEGIN_CLASS4(c, d, e, f) class NET_API c : public d, e, f {
#define BEGIN_CLASS5(c, d, e, f, g) class NET_API c : public d, e, f, g {
#define END_CLASS };
#define CLASS_CONSTRUCTUR(a, ...) a(__VA_ARGS__);
#define CLASS_VCONSTRUCTUR(a, ...) virtual a(__VA_ARGS__);
#define CLASS_DESTRUCTUR(d) ~d();
#define CLASS_VDESTRUCTUR(d) virtual ~d();
#define BEGIN_CLASS_CONSTRUCTUR(a, ...) a(__VA_ARGS__) {
#define BEGIN_CLASS_VCONSTRUCTUR(a, ...) virtual a(__VA_ARGS__) {
#define BEGIN_CLASS_DESTRUCTUR(d) ~d() {
#define BEGIN_CLASS_VDESTRUCTUR(d) virtual ~d() {
#define END_CLASS_CONTRUCTION }
#define CLASS_PRIVATE private:
#define CLASS_PUBLIC public:
#define CLASS_PROTECTED protected:

#define BEGIN_STRUCT(s) struct s {
#define BEGIN_STRUCT2(s, t) struct s : public t {
#define END_STRUCT };
#define STRUCT_CONSTRUCTUR(c) c();
#define STRUCT_VCONSTRUCTUR(c) virtual c();
#define STRUCT_DESTRUCTUR(d) ~d();
#define STRUCT_VDESTRUCTUR(d) virtual ~d();
#define BEGIN_STRUCT_CONSTRUCTUR(c) c() {
#define BEGIN_STRUCT_VCONSTRUCTUR(c) virtual c() {
#define BEGIN_STRUCT_DESTRUCTUR(d) ~d() {
#define BEGIN_STRUCT_VDESTRUCTUR(d) virtual ~d() {
#define END_STRUCT_CONTRUCTION }

#define NET_CALLBACK(type, name, ...) \
type name(__VA_ARGS__) override;

#define NET_DEFINE_CALLBACK(type, name, ...) \
virtual type name(__VA_ARGS__)

#define DEBUG_BREAK __debugbreak();

#define NET_SERVER_BEGIN_DATA_PACKAGE_NATIVE(classname) \
bool classname::CheckDataN(PEER peer, const int id, RPackage pkg) \
{ \
if(!peer) \
	return false; \
switch (id) \
{

#define NET_SERVER_BEGIN_DATA_PACKAGE(classname) \
bool classname::CheckData(CPEER peer, const int id, RPackage pkg) \
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
bool classname::CheckDataN(const int id, RPackage pkg) \
{ \
switch (id) \
{

#define NET_CLIENT_BEGIN_DATA_PACKAGE(classname) \
bool classname::CheckData(const int id, RPackage pkg) \
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

////////////////////////////////////////
//    SECTION - TYPE DEFENITIONS     //
//////////////////////////////////////
typedef unsigned int INDEX;

/////////////////////////////////////////////////
//    SECTION - Allocation & Deallocation     //
///////////////////////////////////////////////
template<class T>
T* Alloc(const size_t size = 1)
{
	T* object = new T[size];
	return object;
}
#define ALLOC Alloc

template <typename T>
static void Free(T*& data)
{
	delete[] data;
	data = nullptr;
}
#define FREE(data) Free(data)

///////////////////////////////////////////////
//    SECTION - Package Prefix & Suffix     //
/////////////////////////////////////////////
#define PACKAGE_HEADER "----- BEGIN PACKAGE -----"
#define PACKAGE_FOOTER "----- END PACKAGE -----"
#define BYTE_PACKAGE_HEADER (byte*)PACKAGE_HEADER
#define BYTE_PACKAGE_FOOTER (byte*)PACKAGE_FOOTER

#define PACKAGE_SIZE_HEADER "----- PACKAGE SIZE BEGIN -----"
#define PACKAGE_SIZE_FOOTER "----- PACKAGE SIZE END -----"
#define BYTE_PACKAGE_SIZE_HEADER (byte*)PACKAGE_SIZE_HEADER
#define BYTE_PACKAGE_SIZE_FOOTER (byte*)PACKAGE_SIZE_FOOTER

// Key is crypted using RSA
#define KEY_HEADER "----- BEGIN KEY -----"
#define KEY_FOOTER "----- END KEY -----"
#define BYTE_KEY_HEADER (byte*)KEY_HEADER
#define BYTE_KEY_FOOTER (byte*)KEY_FOOTER

// IV is crypted using RSA
#define IV_HEADER "----- BEGIN IV -----"
#define IV_FOOTER "----- END IV -----"
#define BYTE_IV_HEADER (byte*)IV_HEADER
#define BYTE_IV_FOOTER (byte*)IV_FOOTER

///////////////////////////////////////////////
//    SECTION - SSL Methode Definitions     //
//////////////////////////////////////////////
struct NET_SSL_METHOD_T
{
	int method;
	char name[32];
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
	{NET_SSL_METHOD::NET_SSL_METHOD_TLS, "TLS"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLS_SERVER, "TLS Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLS_CLIENT, "TLS Client"},
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23, "SSLv23"},
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_SERVER, "SSLv23 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv23_CLIENT, "SSLv23 Client"},
#ifndef OPENSSL_NO_SSL3_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3, "SSLv3"},
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_SERVER, "SSLv3 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_SSLv3_CLIENT, "SSLv3 Client"},
#endif
#ifndef OPENSSL_NO_TLS1_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1, "TLSv1"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_SERVER, "TLSv1 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_CLIENT,"TLSv1 Client"},
#endif
#ifndef OPENSSL_NO_TLS1_1_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1,  "TLSv1_1"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_SERVER,  "TLSv1_1 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_1_CLIENT, "TLSv1_1 Client"},
#endif
#ifndef OPENSSL_NO_TLS1_2_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2, "TLSv1_2"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_SERVER,  "TLSv1_2 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_CLIENT, "TLSv1_2 Client"},
#endif
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLS,  "DTLS"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLS_SERVER, "DTLS Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLS_CLIENT, "DTLS Client"},
#ifndef OPENSSL_NO_DTLS1_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1,  "DTLSv1"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_SERVER, "DTLSv1 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_CLIENT, "DTLSv1 Client"},
#endif
#ifndef OPENSSL_NO_DTLS1_2_METHOD
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2, "DTLSv1_2"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_SERVER, "DTLSv1_2 Server"},
	{NET_SSL_METHOD::NET_SSL_METHOD_DTLSv1_2_CLIENT, "DTLSv1_2 Client"}
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

			case NET_SSL_METHOD::NET_SSL_METHOD_DTLS:
				return DTLS_method();

			case NET_SSL_METHOD::NET_SSL_METHOD_DTLS_SERVER:
				return DTLS_server_method();

			case NET_SSL_METHOD::NET_SSL_METHOD_DTLS_CLIENT:
				return DTLS_client_method();

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
			default:
				break;
			}
		}
	}

	return nullptr;
}

static char* NET_GET_SSL_METHOD_NAME(const int method)
{
	for (auto& val : NET_SSL_METHOD_L)
	{
		if (val.method == method)
			return val.name;
	}

	return (char*)"UNKNOWN";
}

//////////////////////////////
//    USEFULL FUNCTIONS    //
////////////////////////////
static bool NET_STRING_IS_NUMBER(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}