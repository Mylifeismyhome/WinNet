#pragma once

/*
Legend of Variable Names:
 s => Settings
*/

#include <Net/Net.h>
#include <Net/Package.h>
#include "Net/NetCodes.hpp"

#define NET_SERVER Net::Server::Server

// Typedef in style of Define
#define PEERSTABLE vPeers

#define IPEER peerInfo
#define PEER peerInfo*
#define CPEER const peerInfo*

#define IASYNCPEER AsyncPeers
#define ASYNCPEER AsyncPeers*
#define CASYNCPEER const AsyncPeer*

#define const_cast_Peer ConstCastPeer

#include <Cryption/AES.h>
#include <Cryption/RSA.h>
#include <Coding/MD5.h>
#include <Coding/BASE64.h>
#include <Coding/SHA1.h>
#include <Compression/Compression.h>

#include <ICMP/icmp.h>

#ifdef _WIN64
typedef DWORD64 lt;
#else
typedef DWORD lt;
#endif

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Server)
BEGIN_CLASS2(Server, Package)
CLASS_PUBLIC
#pragma region PEERS TABLE
// table to keep track of each client's socket
BEGIN_STRUCT(IPEER)
explicit operator bool() const
{
	return (nullptr ? false : UniqueID == 0 ? false : true);
}

// unique identification ID
DWORD UniqueID;
SOCKET pSocket;
struct sockaddr_in client_addr;
float lastaction;

bool estabilished;

/* network data */
byte* data;
size_t data_size;
size_t data_full_size;
bool recordingData;
char* RSAPublicKey; // client Public Key
char* RSAPrivateKey; // own Private Key
bool RSAHandshake; // set to true as soon as we have the public key from the Peer

/* Async Handler */
unsigned int Async;
bool OnAsyncList;

/* Net Version */
bool NetVersionMatched;

lt latency;
double lastCalcLatency;

BEGIN_STRUCT_CONSTRUCTUR(IPEER)
UniqueID = 0;
pSocket = NULL;
client_addr = sockaddr_in();
lastaction = 0.0f;
estabilished = false;
data = nullptr;
data_size = 0;
data_full_size = 0;
recordingData = false;
RSAPublicKey = nullptr;
RSAPrivateKey = nullptr;
RSAHandshake = false;
Async = 0;
OnAsyncList = false;
NetVersionMatched = false;
latency = -1;
lastCalcLatency = 0;
END_STRUCT_CONTRUCTION

void clear();
void AllocData(const unsigned __int64);
void clearData();
void createNewRSAKeys(const unsigned short);
void deleteRSAKeys();
lt getLatency() const;
END_STRUCT

CLASS_PRIVATE
IPEER* vPeers;

CLASS_PUBLIC
PEER GetPeer(const DWORD) const;
PEER GetPeer(const SOCKET) const;
PEER GetPeer(CPEER) const;
PEER GetPeer(PEER) const;
PEER GetPeerByIndex(const INDEX) const;
const PEER ConstCastPeer(PEER);
void SetPeerAsync(PEER, unsigned int);
void SetPeerDataRecording(PEER, bool);
void DisconnectPeer(PEER, int);
void DisconnectPeer(CPEER, int);
void DisconnectPeer(const SOCKET, int);
#pragma endregion
#pragma region ASYNC PEERS TABLE
BEGIN_STRUCT(AsyncPeers)
explicit operator bool() const
{
	return (nullptr ? false : UniqueID == 0 ? false : true);
}
DWORD UniqueID;
int func;

BEGIN_STRUCT_CONSTRUCTUR(AsyncPeers)
UniqueID = 0;
func = -1;
END_STRUCT_CONTRUCTION

void clear();
END_STRUCT

CLASS_PRIVATE
IASYNCPEER* vAsyncPeers;

CLASS_PUBLIC
ASYNCPEER GetAsyncPeer(const DWORD) const;
#pragma endregion
CLASS_PRIVATE
long GetFreeSlot() const;
long GetFreeAsyncSlot() const;
DWORD GetFreeUnqiueID() const;
bool InsertPeer(struct sockaddr_in);
bool ErasePeer(PEER);
bool AddPeerToAsyncList(PEER, const int) const;
void UpdatePeer(PEER);

size_t GetNextPackageSize(PEER) const;
size_t GetReceivedPackageSize(PEER) const;
float GetReceivedPackageSizeAsPerc(PEER) const;

char sServerName[128];
u_short sServerPort;
DWORD sfrequenz;
float sShutdownTimer;
u_short sMaxThreads;
float sTimeSpamProtection;
unsigned int sMaxPeers;
u_short sRSAKeySize;
u_short sAESKeySize;
bool sCryptPackage;
bool sCompressPackage;
int sMaxPackageSize;
u_short sShutdownKey;
long sTCPReadTimeout;
long sCalcLatencyInterval;

CLASS_PUBLIC
void SetAllToDefault();
void SetServerName(const char*);
void SetServerPort(u_short);
void SetFrequenz(DWORD);
void SetShutdownTimer(float);
void SetMaxThreads(u_short);
void SetTimeSpamProtection(float);
void SetMaxPeers(unsigned int);
void SetRSAKeySize(u_short);
void SetAESKeySize(u_short);
void SetCryptPackage(bool);
void SetCompressPackage(bool);
void SetMaxPackageSize(int);
void SetShutdownKey(u_short);
void SetTCPReadTimeout(long);
void SetCalcLatencyInterval(long);

const char* GetServerName() const;
u_short GetServerPort() const;
DWORD GetFrequenz() const;
float GetShutdownTimer() const;
u_short GetMaxThreads() const;
float GetTimeSpamProtection() const;
unsigned int GetMaxPeers() const;
u_short GetRSAKeySize() const;
u_short GetAESKeySize() const;
bool GetCryptPackage() const;
bool GetCompressPackage() const;
int GetMaxPackageSize() const;
u_short GetShutdownKey() const;
long GetTCPReadTimeout() const;
long GetCalcLatencyInterval() const;

CLASS_PRIVATE
SOCKET ListenSocket;
SOCKET AcceptSocket;

bool DoExit;
bool bRunning;
bool bShuttingDown;
bool DoShutdown;

CLASS_PUBLIC
void SetListenSocket(const SOCKET);
void SetAcceptSocket(const SOCKET);
void SetRunning(const bool);
void SetShutdown(const bool);

SOCKET GetListenSocket() const;
SOCKET GetAcceptSocket() const;
bool IsRunning() const;
bool IsShutdown() const;

CLASS_CONSTRUCTUR(Server)
CLASS_VDESTRUCTUR(Server)
bool Start(const char*, u_short);
bool Close();
void Terminate();

CLASS_PRIVATE
void BaseTick();
void Acceptor();
void ProcessAsyncPeer(PEER);

CLASS_PUBLIC
NET_DEFINE_CALLBACK(void, Tick) {}
NET_DEFINE_CALLBACK(bool, CheckData, CPEER peer, int id, RPackage pkg) { return false; }
bool NeedExit() const;
void Shutdown();

void DoSend(const SOCKET, const int, RPackage);
void DoSend(PEER, const int, RPackage);
void DoSend(CPEER, const int, RPackage);
void Broadcast(const int, RPackage);

unsigned int GetCountConnectedPeers() const;

CLASS_PRIVATE
void ReceiveThread(PEER);
void BaseTickThread();
void AcceptorThread();
void UpdatePeersThread(PEER);
void LatencyThread(PEER);
void DoReceive(SOCKET);
void DoReceive(PEER);
void ProcessPackage(PEER, BYTE*, size_t);

void CheckForShutdown();

bool Handshake(SOCKET) const;

bool CheckDataN(PEER peer, int id, RPackage pkg);

/* Native Packages */
void OnRSAHandshake(PEER, RPackage);
void OnVersionPackage(PEER, RPackage);

void AllocPeersTable();
void DeallocPeersTable();
void AllocAsyncManager();
void DeallocAsyncManager();

CLASS_PROTECTED
/* CALLBACKS */
NET_DEFINE_CALLBACK(void, OnPeerConnect, CPEER) {}
NET_DEFINE_CALLBACK(void, OnPeerDisconnect, CPEER) {}
NET_DEFINE_CALLBACK(void, OnPeerEstabilished, CPEER) {}
NET_DEFINE_CALLBACK(void, PeerThreadTick, CPEER) {}
END_CLASS

inline bool operator==(const NET_SERVER::IPEER peer1, const NET_SERVER::IPEER peer2)
{
	return (peer1.UniqueID == peer2.UniqueID);
}
END_NAMESPACE
END_NAMESPACE