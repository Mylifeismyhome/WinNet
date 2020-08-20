#pragma once
#include <Net/Net.h>
#include <Net/Package.h>
#include <Net/NetCodes.hpp>

#define NET_CLIENT Net::Client::Client

#include <Cryption/AES.h>
#include <Cryption/RSA.h>
#include <Compression/Compression.h>

#include <ICMP/icmp.h>

#ifdef _WIN64
typedef DWORD64 lt;
#else
typedef DWORD lt;
#endif

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Client)
BEGIN_CLASS2(Client, Package)
BEGIN_STRUCT(Network)
byte* data;
size_t data_size;
size_t data_full_size;
bool recordingData;
char* RSAPublicKey; // Server Public Key
char* RSAPrivateKey; // own Private Key
bool RSAHandshake; // set to true as soon as we have the public key from the Server

bool estabilished;

lt latency;
double lastCalcLatency;

BEGIN_STRUCT_CONSTRUCTUR(Network)
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
END_STRUCT_CONTRUCTION

void clear();
void AllocData(const size_t);
void clearData();
void createNewRSAKeys(const unsigned short);
void deleteRSAKeys();
lt getLatency() const;
END_STRUCT
CLASS_PUBLIC
Network network;

CLASS_PRIVATE
DWORD sfrequenz;
int sMaxPackageSize;
bool sBlockingMode;
u_short sMaxThreads;
u_short sRSAKeySize;
u_short sAESKeySize;
bool sCryptPackage;
bool sCompressPackage;
long sCalcLatencyInterval;

CLASS_PUBLIC
void SetAllToDefault();
void SetFrequenz(DWORD);
void SetMaxPackageSize(int);
void SetBlockingMode(bool);
void SetMaxThreads(u_short);
void SetRSAKeySize(u_short);
void SetAESKeySize(u_short);
void SetCryptPackage(bool);
void SetCompressPackage(bool);
void SetCalcLatencyInterval(long);

DWORD GetFrequenz() const;
int GetMaxPackageSize() const;
bool GetBlockingMode() const;
u_short GetMaxThreads() const;
u_short GetRSAKeySize() const;
u_short GetAESKeySize() const;
bool GetCryptPackage() const;
bool GetCompressPackage() const;
long GetCalcLatencyInterval() const;

CLASS_PRIVATE
bool NeedExit;
SOCKET csocket;
struct sockaddr_in csocketAddr;
char* ServerAddress;
u_short ServerPort;
bool connected;
bool KeysSet;

void SetRecordingData(bool);

void Timeout();

/* clear all stored data */
void ConnectionClosed();

void ReceiveThread();
void BaseTickThread();

CLASS_PUBLIC
CLASS_CONSTRUCTUR(Client)
CLASS_VDESTRUCTUR(Client)

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
bool GetConnected() const;

void SetKeysSet(bool);
bool GetKeysSet() const;

size_t GetNextPackageSize() const;
size_t GetReceivedPackageSize() const;
float GetReceivedPackageSizeAsPerc() const;

void BaseTick();
void LatencyTick();
NET_DEFINE_CALLBACK(void, Tick) {}
void Packager();
bool CheckDataN(int id, RPackage pkg);
NET_DEFINE_CALLBACK(bool, CheckData, const int id, RPackage pkg) { return false; }

void DoSend(const int, RPackage);

CLASS_PRIVATE
void DoReceive();
void ProcessPackage(BYTE*, size_t);

void OnRSAHandshake(RPackage pkg);
void OnKeysPackage(const RPackage);
void OnVersionPackage(RPackage);
void OnEstabilishConnectionPackage(const RPackage);
void OnClosePackage(const RPackage);

CLASS_PROTECTED
// Callback
NET_DEFINE_CALLBACK(void, OnConnected) {}
NET_DEFINE_CALLBACK(void, OnDisconnected) {}
NET_DEFINE_CALLBACK(void, OnForcedDisconnect, int code) {}
NET_DEFINE_CALLBACK(void, OnTimeout) {}
NET_DEFINE_CALLBACK(void, OnKeysReceived) {}
NET_DEFINE_CALLBACK(void, OnKeysFailed) {}
NET_DEFINE_CALLBACK(void, OnConnectionEstabilished) {}
NET_DEFINE_CALLBACK(void, OnVersionMismatch) {}
END_CLASS
END_NAMESPACE
END_NAMESPACE