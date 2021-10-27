#pragma once
#define NET_HTTP Net::Web::HTTP
#define NET_HTTPS Net::Web::HTTPS
#define NET_USER_AGENT CSTRING("Net.Net/100")

#include <Net/Net/Net.h>
#include <Net/assets/assets.h>
#include <openssl/err.h>

#ifdef BUILD_LINUX
#define LAST_ERROR errno
#else
#define LAST_ERROR WSAGetLastError()
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Web)
NET_DSA_BEGIN
NET_STRUCT_BEGIN(HeaderData_t)
char* key;
char* value;

explicit HeaderData_t(const char*, char*, size_t = INVALID_SIZE);
explicit HeaderData_t(const char*, const char*, size_t = INVALID_SIZE);
explicit HeaderData_t(const char*, unsigned char*, size_t = INVALID_SIZE);

void free();
NET_STRUCT_END

NET_CLASS_BEGIN(Head)
NET_STRUCT_BEGIN(Network)
byte dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
CPOINTER<byte> data;
size_t data_size;
size_t data_full_size;

NET_STRUCT_BEGIN_CONSTRUCTUR(Network)
memset(dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
data = nullptr;
data_size = 0;
data_full_size = 0;
NET_STRUCT_END_CONTRUCTION

void AllocData(size_t);
void clearData();
NET_STRUCT_END
NET_CLASS_PROTECTED
std::string protocol;
std::string url;
std::string path;
short port;
Network network;
std::vector<HeaderData_t> headerData;

SOCKET connectSocket;
struct addrinfo *connectSocketAddr;

std::map<std::string, int>INT_Parameters;
std::map<std::string, long>LONG_Parameters;
std::map<std::string, long long>LONGLONG_Parameters;
std::map<std::string, float>FLOAT_Parameters;
std::map<std::string, const char*>STRING_Parameters;
std::string STRING_JSON;
std::string GetParameters() const;

bool ParseResult();

std::string rawData;
std::string headContent;
std::string bodyContent;
int resultCode;

void ShutdownSocket(int = SOCKET_RDWR) const;

void SetRawData(std::string);
void SetHeaderContent(std::string);
void SetBodyContent(std::string);
void SetResultCode(int);

SOCKET GetSocket() const;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(Head)
NET_CLASS_DESTRUCTUR(Head)

std::string& GetProtocol();
std::string& GetURL();
std::string& GetPath();
short GetPort() const;

std::string& GetHeaderContent();
std::string& GetBodyContent();
std::string& GetRawData();
int GetResultCode() const;

void URL_Encode(char*&) const;
void URL_Encode(std::string&) const;
void URL_Decode(char*&) const;
void URL_Decode(std::string&) const;

void AddParam(const char*, int);
void AddParam(const char*, long);
void AddParam(const char*, long long);
void AddParam(const char*, float);
void AddParam(const char*, const char*);
void SetJSON(const char*);

void AddHeader(const char*, char*, size_t = INVALID_SIZE);
void AddHeader(const char*, const char*, size_t = INVALID_SIZE);
void AddHeader(const char*, unsigned char*, size_t = INVALID_SIZE);
NET_CLASS_END

NET_ABSTRAC_CLASS_BEGIN(HTTP, Head)
friend class Head;

bool Init(const char*);
bool Inited;

size_t DoSend(std::string&) const;
size_t DoReceive();
NET_CLASS_PUBLIC
explicit NET_CLASS_CONSTRUCTUR(HTTP, const char*)
NET_CLASS_DESTRUCTUR(HTTP)

bool IsInited() const;

// Methodes
bool Get();
bool Post();
NET_CLASS_END

NET_ABSTRAC_CLASS_BEGIN(HTTPS, Head)
friend class Head;
SSL_CTX* ctx;
SSL* ssl;

bool Init(const char*, ssl::NET_SSL_METHOD);
bool Inited;

size_t DoSend(std::string&) const;
size_t DoReceive();
NET_CLASS_PUBLIC
explicit NET_CLASS_CONSTRUCTUR(HTTPS, const char*, ssl::NET_SSL_METHOD = ssl::NET_SSL_METHOD::NET_SSL_METHOD_TLS)
NET_CLASS_DESTRUCTUR(HTTPS)

bool IsInited() const;

// Methodes
bool Get();
bool Post();
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END
