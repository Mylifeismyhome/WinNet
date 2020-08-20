#pragma once
/*
Legend of Variable Names:
 s => Settings
 h => Handshake
*/

#include <Net/Net.h>
#include <Net/Package.h>
#include <Net/NetCodes.hpp>

#define NET_WEB_SERVER Net::WebServer::Server

// Typedef in style of Define
#define PEERSTABLE vPeers

#define IPEER peerInfo
#define PEER peerInfo*
#define CPEER const peerInfo*

#define IASYNCPEER AsyncPeers
#define ASYNCPEER AsyncPeers*
#define CASYNCPEER const AsyncPeer*

#define const_cast_Peer ConstCastPeer

#define OPCODE_CONTINUE 0x0
#define OPCODE_TEXT 0x1
#define OPCODE_BINARY 0x2
#define OPCODE_CLOSE 0x8
#define OPCODE_PING 0x9
#define OPCODE_PONG 0xA

#include <Cryption/AES.h>
#include <Cryption/RSA.h>
#include <Coding/MD5.h>
#include <Coding/BASE64.h>
#include <Coding/SHA1.h>
#include <Compression/Compression.h>

#include <OpenSSL/include/openssl/ssl.h>
#include <OpenSSL/include/openssl/err.h>

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(WebServer)
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

/* Async Handler */
unsigned int Async;
bool OnAsyncList;

/* SSL */
SSL* ssl;

/* Handshake */
bool handshake;

BEGIN_STRUCT_CONSTRUCTUR(IPEER)
UniqueID = 0;
pSocket = NULL;
client_addr = sockaddr_in();
lastaction = 0.0f;
estabilished = false;
data = nullptr;
data_size = 0;
Async = 0;
OnAsyncList = false;
ssl = nullptr;
handshake = false;
END_STRUCT_CONTRUCTION

void clear();
void AllocData(const unsigned __int64);
void clearData();
END_STRUCT

CLASS_PRIVATE
IPEER* vPeers;

CLASS_PUBLIC
PEER GetPeer(const DWORD) const;
PEER GetPeer(const SOCKET) const;
PEER GetPeer(CPEER) const;
PEER GetPeer(PEER) const;
PEER GetPeerByIndex(const INDEX) const;
PEER ConstCastPeer(PEER) const;
void SetPeerAsync(PEER, const unsigned int);
void SetPeerDataRecording(PEER, const bool);
void DisconnectPeer(PEER, int);
void DisconnectPeer(CPEER, int);
void DisconnectPeer(SOCKET, int);
#pragma endregion
#pragma region ASYNC PEERS TABLE
BEGIN_STRUCT(AsyncPeers)
explicit operator bool() const
{
	return (nullptr ? false : UniqueID == -1 ? false : true);
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
bool InsertPeer(const sockaddr_in);
bool ErasePeer(PEER);
bool AddPeerToAsyncList(PEER, const int) const;
void UpdatePeer(PEER);

char sServerName[128];
u_short sServerPort;
DWORD sfrequenz;
float sShutdownTimer;
u_short sMaxThreads;
float sTimeSpamProtection;
unsigned int sMaxPeers;
bool sSSL;
char sCertFileName[MAX_PATH];
char sKeyFileName[MAX_PATH];
char sCaFileName[MAX_PATH];
bool hUseCustom;
char* hOrigin;
bool sCompressPackage;
int sMaxPackageSize;
u_short sShutdownKey;
long sTCPReadTimeout;

CLASS_PUBLIC
void SetAllToDefault();
void SetServerName(const char*);
void SetServerPort(u_short);
void SetFrequenz(DWORD);
void SetShutdownTimer(float);
void SetMaxThreads(u_short);
void SetTimeSpamProtection(float);
void SetMaxPeers(unsigned int);
void SetSSL(bool);
void SetCertFileName(const char*);
void SetKeyFileName(const char*);
void SetCaFileName(const char*);
void SetCustomHandshakeMethode(bool);
void SetHandshakeOriginCompare(const char*);
void SetCompressPackage(bool);
void SetMaxPackageSize(int);
void SetShutdownKey(u_short);
void SetTCPReadTimeout(long);

const char* GetServerName() const;
u_short GetServerPort() const;
DWORD GetFrequenz() const;
float GetShutdownTimer() const;
u_short GetMaxThreads() const;
float GetTimeSpamProtection() const;
unsigned int GetMaxPeers() const;
bool GetSSL() const;
const char* GetCertFileName() const;
const char* GetKeyFileName() const;
const char* GetCaFileName() const;
bool GetCustomHandshakeMethode() const;
const char* GetHandshakeOriginCompare() const;
bool GetCompressPackage() const;
int GetMaxPackageSize() const;
u_short GetShutdownKey() const;
long GetTCPReadTimeout() const;

CLASS_PRIVATE
SOCKET ListenSocket;
SOCKET AcceptSocket;

bool DoExit;
bool bRunning;
bool bShuttingDown;

CLASS_PUBLIC
void SetListenSocket(SOCKET);
void SetAcceptSocket(SOCKET);
void SetRunning(bool);
void SetShutdown(bool);

SOCKET GetListenSocket() const;
SOCKET GetAcceptSocket() const;
bool IsRunning() const;
bool IsShutdown() const;
bool DoShutdown;

CLASS_PRIVATE
SSL_CTX* ctx;
short Handshake(CPEER);
void onSSLTimeout(const SOCKET);
void onSSLTimeout(CPEER);

bool CheckDataN(PEER peer, int id, RPackage pkg);

CLASS_PUBLIC
CLASS_CONSTRUCTUR(Server)
CLASS_VDESTRUCTUR(Server)
bool Start(const char*, u_short, NET_SSL_METHOD = NET_SSL_METHOD::NET_SSL_METHOD_TLS);
bool Close();
void Terminate();

CLASS_PRIVATE
void BaseTick();
void Acceptor();
void ProcessAsyncPeer(PEER);

CLASS_PUBLIC
NET_DEFINE_CALLBACK(void, Tick) {}
NET_DEFINE_CALLBACK(bool, CheckData, CPEER peer, const int id, RPackage pkg) { return false; }
bool NeedExit() const;
void Shutdown();

void DoSend(const SOCKET, const int, RPackage);
void DoSend(CPEER, const int, RPackage);
void Broadcast(const int, RPackage);

unsigned int GetCountConnectedPeers() const;

CLASS_PRIVATE
short ThreadsRunning;
void ReceiveThread(PEER);
void BaseTickThread();
void AcceptorThread();
void UpdatePeersThread(PEER);
void DoReceive(SOCKET);
void DoReceive(PEER);
int GetBitFromDataFrame(const BYTE*, unsigned int) const;
void ProcessPackage(PEER, BYTE*, size_t);

void CheckForShutdown();

void AllocPeersTable();
void DeallocPeersTable();
void AllocAsyncManager();
void DeallocAsyncManager();

CLASS_PROTECTED
/* CALLBACKS */
NET_DEFINE_CALLBACK(void, OnPeerConnect, CPEER) {}
NET_DEFINE_CALLBACK(void, OnPeerDisconnect, CPEER) {}
NET_DEFINE_CALLBACK(void, PeerThreadTick, CPEER) {}
END_CLASS

inline bool operator==(const NET_WEB_SERVER::IPEER peer1, const NET_WEB_SERVER::IPEER peer2)
{
	return (peer1.UniqueID == peer2.UniqueID);
}
END_NAMESPACE
END_NAMESPACE