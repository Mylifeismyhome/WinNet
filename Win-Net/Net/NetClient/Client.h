#pragma once
#define NET_CLIENT Net::Client::Client

#define PKG pkg
#define FUNCTION_NAME CASTRING("On" __FUNCTION__)
#define NET_BEGIN_FUNC_PACKAGE(cs, fnc) void cs::On##fnc(NET_PACKAGE PKG) {

#define NET_END_FUNC_PACKAGE }
#define NET_BEGIN_FNC_PKG NET_BEGIN_FUNC_PACKAGE
#define NET_END_FNC_PKG NET_END_FUNC_PACKAGE
#define NET_DEF_FUNC_PACKAGE(fnc) void On##fnc(NET_PACKAGE)
#define NET_DEF_FNC_PKG NET_DEF_FUNC_PACKAGE

#define NET_SEND DoSend

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <Net/Net/NetCodes.h>
#include <Net/Net/NetVersion.h>

#include <Cryption/AES.h>
#include <Cryption/RSA.h>
#include <Compression/Compression.h>
#include <Cryption/PointerCryption.h>

#include <ICMP/icmp.h>

constexpr auto DEFAULT_MAX_THREADS = 2;
constexpr auto DEFAULT_RSA_KEY_SIZE = 1024;
constexpr auto DEFAULT_AES_KEY_SIZE = CryptoPP::AES::MAX_KEYLENGTH;
constexpr auto DEFAULT_CRYPT_PACKAGES = false;
constexpr auto DEFAULT_COMPRESS_PACKAGES = false;
constexpr auto DEFAULT_BLOCKING_MODE = false;
constexpr auto DEFAULT_FREQUENZ = 66;
constexpr auto DEFAULT_MAX_PACKET_SIZE = 65536;
constexpr auto DEFAULT_CALC_LATENCY_INTERVAL = 10; // Seconds

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Client)
NET_DSA_BEGIN
NET_STRUCT_BEGIN(rawData_t)
BYTE* data;
size_t size;

rawData_t(BYTE* pointer, const size_t size)
{
	data = pointer;
	this->size = size;
}
NET_STRUCT_END

NET_ABSTRAC_CLASS_BEGIN(Client, Package)
NET_STRUCT_BEGIN(Network)
byte dataReceive[DEFAULT_MAX_PACKET_SIZE];
CPOINTER<byte> data;
size_t data_size;
size_t data_full_size;
bool recordingData;
NET_RSA* RSA;
bool RSAHandshake; // set to true as soon as we have the public key from the Server

bool estabilished;

typeLatency latency;
double lastCalcLatency;

NET_STRUCT_BEGIN_CONSTRUCTUR(Network)
memset(dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
data = nullptr;
data_size = 0;
data_full_size = 0;
recordingData = false;
RSA = nullptr;
RSAHandshake = false;
estabilished = false;
latency = -1;
lastCalcLatency = 0;
NET_STRUCT_END_CONTRUCTION

void clear();
void AllocData(size_t);
void clearData();
void createNewRSAKeys(size_t);
void deleteRSAKeys();
typeLatency getLatency() const;
NET_STRUCT_END
NET_CLASS_PUBLIC
Network network;

NET_CLASS_PRIVATE
long long sfrequenz;
bool sBlockingMode;
u_short sMaxThreads;
size_t sRSAKeySize;
size_t sAESKeySize;
bool sCryptPackage;
bool sCompressPackage;
long sCalcLatencyInterval;
std::vector<SocketOption_t<char*>> socketoption;

NET_CLASS_PUBLIC
void SetAllToDefault();
void SetFrequenz(long long);
void SetBlockingMode(bool);
void SetMaxThreads(u_short);
void SetRSAKeySize(size_t);
void SetAESKeySize(size_t);
void SetCryptPackage(bool);
void SetCompressPackage(bool);
void SetCalcLatencyInterval(long);
void SetSocketOption(DWORD, bool);

template <class T>
void SetSocketOption(const SocketOption_t<T> opt)
{
	SocketOption_t<char*> option;
	option.opt = opt.opt;
	option.type = reinterpret_cast<char*>(opt.type);
	option.len = opt.len;
	socketoption.emplace_back(option);
}

long long GetFrequenz() const;
bool GetBlockingMode() const;
u_short GetMaxThreads() const;
size_t GetRSAKeySize() const;
size_t GetAESKeySize() const;
bool GetCryptPackage() const;
bool GetCompressPackage() const;
long GetCalcLatencyInterval() const;

NET_CLASS_PRIVATE
bool NeedExit;
SOCKET connectSocket;
struct addrinfo* connectSocketAddr;
CPOINTER<char> ServerAddress;
u_short ServerPort;
bool connected;
bool KeysSet;

void SetRecordingData(bool);

void Timeout();

/* clear all stored data */
void ConnectionClosed();

void ReceiveThread();
void BaseTickThread();

void CompressData(BYTE*&, size_t&) const;
void DecompressData(BYTE*&, size_t&) const;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(Client)
NET_CLASS_VDESTRUCTUR(Client)

bool ChangeMode(const bool) const;

bool Connect(const char*, u_short);
bool Disconnect();
void Terminate();
void Clear();
bool DoNeedExit() const;

void SetSocket(const SOCKET);
SOCKET GetSocket() const;

void SetServerAddress(const char*);
const char* GetServerAddress() const;

void SetServerPort(u_short);
u_short GetServerPort() const;

void SetConnected(bool);
bool IsConnected() const;

void SetKeysSet(bool);
bool GetKeysSet() const;

size_t GetNextPackageSize() const;
size_t GetReceivedPackageSize() const;
float GetReceivedPackageSizeAsPerc() const;

void BaseTick();
void LatencyTick();
NET_DEFINE_CALLBACK(void, Tick) {}
void Packager();
bool CheckDataN(int id, NET_PACKAGE pkg);
NET_DEFINE_CALLBACK(bool, CheckData, const int id, NET_PACKAGE pkg) { return false; }

void SingleSend(const char*, size_t, bool&);
void SingleSend(BYTE*&, size_t, bool&);
void SingleSend(CPOINTER<BYTE>&, size_t, bool&);
void DoSend(int, NET_PACKAGE);

NET_CLASS_PRIVATE
void DoReceive();
void GetPackageDataSize();
void ProcessPackages();
void ExecutePackage(size_t, size_t);

NET_DEF_FNC_PKG(RSAHandshake);
NET_DEF_FNC_PKG(KeysPackage);
NET_DEF_FNC_PKG(VersionPackage);
NET_DEF_FNC_PKG(EstabilishConnectionPackage);
NET_DEF_FNC_PKG(ClosePackage);

NET_CLASS_PROTECTED
// Callback
NET_DEFINE_CALLBACK(void, OnConnected) {}
NET_DEFINE_CALLBACK(void, OnDisconnected) {}
NET_DEFINE_CALLBACK(void, OnForcedDisconnect, int code) {}
NET_DEFINE_CALLBACK(void, OnTimeout) {}
NET_DEFINE_CALLBACK(void, OnKeysReceived) {}
NET_DEFINE_CALLBACK(void, OnKeysFailed) {}
NET_DEFINE_CALLBACK(void, OnConnectionEstabilished) {}
NET_DEFINE_CALLBACK(void, OnVersionMismatch) {}
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END