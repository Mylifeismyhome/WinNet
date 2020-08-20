#pragma once
#include <Net/Net.h>
#include <Net/Package.h>
#include <Net/NetCodes.hpp>

#define NET_CLIENT Net::Client::Client

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
constexpr auto DEFAULT_MAX_PACKET_SIZE = 65535;
constexpr auto DEFAULT_CALC_LATENCY_INTERVAL = 10; // Seconds

NET_DSA_BEGIN

#ifdef _WIN64
typedef DWORD64 lt;
#else
typedef DWORD lt;
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Client)
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
CPOINTER<char> RSAPublicKey; // Server Public Key
CPOINTER<char> RSAPrivateKey; // own Private Key
bool RSAHandshake; // set to true as soon as we have the public key from the Server

bool estabilished;

lt latency;
double lastCalcLatency;

NET_STRUCT_BEGIN_CONSTRUCTUR(Network)
memset(dataReceive, NULL, DEFAULT_MAX_PACKET_SIZE);
data = nullptr;
data_size = 0;
data_full_size = 0;
recordingData = false;
RSAPublicKey = nullptr;
RSAPrivateKey = nullptr;
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
lt getLatency() const;
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
SOCKET csocket;
struct sockaddr_in csocketAddr;
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

void CompressData(BYTE**, size_t&) const;
void DecompressData(BYTE**, size_t&) const;

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

void SingleSend(const char*, size_t);
void SingleSend(BYTE*&, size_t);
void SingleSend(CPOINTER<BYTE>&, size_t);
void DoSend(int, NET_PACKAGE);

NET_CLASS_PRIVATE
void DoReceive();
void GetPackageDataSize();
void ProcessPackages();
void ExecutePackage(size_t, size_t);

void OnRSAHandshake(NET_PACKAGE pkg);
void OnKeysPackage(NET_PACKAGE);
void OnVersionPackage(NET_PACKAGE);
void OnEstabilishConnectionPackage(NET_PACKAGE);
void OnClosePackage(NET_PACKAGE);

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
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END