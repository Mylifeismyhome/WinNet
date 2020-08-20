#pragma once
#include <Net/Net.h>

#define NET_HTTP Net::web::HTTP
#define NET_HTTPS Net::web::HTTPS

#include <assets/assets.h>

#include <OpenSSL/include/openssl/ssl.h>
#include <OpenSSL/include/openssl/err.h>

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(web)
BEGIN_CLASS(HTTPHead)
CLASS_PROTECTED
std::string protocol;
std::string url;
std::string path;
int port;

int BufferSize;

SOCKET connectSocket;
struct sockaddr_in connectSocketAddr;

std::map<std::string, int>INT_Parameters;
std::map<std::string, long>LONG_Parameters;
std::map<std::string, long long>LONGLONG_Parameters;
std::map<std::string, float>FLOAT_Parameters;
std::map<std::string, const char*>STRING_Parameters;
std::string GetParameters() const;

bool ParseResult(byte*&);

std::string rawData;
std::string headContent;
std::string bodyContent;
int resultCode;

void ShutdownSocket() const;

void SetRawData(std::string&);
void SetHeaderContent(std::string&);
void SetBodyContent(std::string&);
void SetResultCode(int);

SOCKET GetSocket() const;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(HTTPHead)
CLASS_DESTRUCTUR(HTTPHead)

void SetBufferSize(int);
int GetBufferSize() const;

std::string& GetProtocol();
std::string& GetURL();
std::string& GetPath();
int GetPort() const;

std::string& GetHeaderContent();
std::string& GetBodyContent();
std::string& GetRawData();
int GetResultCode() const;

void URL_Encode(char*&) const;
void URL_Encode(std::string&);
void URL_Decode(char*&) const;
void URL_Decode(std::string&) const;

void AddParam(const char*, int);
void AddParam(const char*, long);
void AddParam(const char*, long long);
void AddParam(const char*, float);
void AddParam(const char*, const char*);
END_CLASS

BEGIN_CLASS2(HTTP, HTTPHead)
friend class HTTPHead;

bool Init(std::string&);
bool Inited;

size_t DoSend(std::string&) const;
size_t DoReceive(byte*&) const;
CLASS_PUBLIC
CLASS_CONSTRUCTUR(HTTP, std::string)
CLASS_DESTRUCTUR(HTTP)

bool IsInited() const;

// Methodes
bool Get();
bool Post();
END_CLASS

BEGIN_CLASS2(HTTPS, HTTPHead)
friend class HTTPHead;
SSL_CTX* ctx;
SSL* ssl;

bool Init(std::string&, NET_SSL_METHOD);
bool Inited;

size_t DoSend(std::string&) const;
size_t DoReceive(byte*&) const;
CLASS_PUBLIC
CLASS_CONSTRUCTUR(HTTPS, std::string, NET_SSL_METHOD = NET_SSL_METHOD::NET_SSL_METHOD_TLS)
CLASS_DESTRUCTUR(HTTPS)

bool IsInited() const;

// Methodes
bool Get();
bool Post();
END_CLASS
END_NAMESPACE
END_NAMESPACE
