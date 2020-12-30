#pragma once
#define NET_SERVER Net::Server::Server

#define NET_IPEER peerInfo
#define NET_PEER peerInfo*

#define PEER peer
#define PKG pkg
#define FUNCTION_NAME NET_FUNCTIONNAME
#define NET_BEGIN_FUNC_PACKAGE(cs, fnc) void cs::On##fnc(NET_PEER PEER, NET_PACKAGE PKG) { \
	const char* NET_FUNCTIONNAME = CASTRING("On"#fnc);

#define NET_END_FUNC_PACKAGE }
#define NET_BEGIN_FNC_PKG NET_BEGIN_FUNC_PACKAGE
#define NET_END_FNC_PKG NET_END_FUNC_PACKAGE
#define NET_DEF_FUNC_PACKAGE(fnc) void On##fnc(NET_PEER, NET_PACKAGE)
#define NET_DEF_FNC_PKG NET_DEF_FUNC_PACKAGE

#define NET_SEND DoSend

#define NET_PEER_WAIT_LOCK(peer) while (peer && peer->bQueueLock) {};

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <Net/Net/NetCodes.h>
#include <Net/Net/NetVersion.h>

#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>
#include <Net/Coding/MD5.h>
#include <Net/Coding/BASE64.h>
#include <Net/Coding/SHA1.h>
#include <Net/Compression/Compression.h>

#include <Net/Protocol/ICMP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>

/* DEFAULT SETTINGS AS MACRO */
CONSTEXPR auto DEFAULT_SERVER_SERVERNAME = "UNKNOWN";
CONSTEXPR auto DEFAULT_SERVER_SERVERPORT = 50000;
CONSTEXPR auto DEFAULT_SERVER_FREQUENZ = 30;
CONSTEXPR auto DEFAULT_SERVER_RSA_KEY_SIZE = 1024;
CONSTEXPR auto DEFAULT_SERVER_AES_KEY_SIZE = CryptoPP::AES::MAX_KEYLENGTH;
CONSTEXPR auto DEFAULT_SERVER_CRYPT_PACKAGES = false;
CONSTEXPR auto DEFAULT_SERVER_COMPRESS_PACKAGES = false;
CONSTEXPR auto DEFAULT_SERVER_MAX_PACKET_SIZE = 512;
CONSTEXPR auto DEFAULT_SERVER_TCP_READ_TIMEOUT = 10;
CONSTEXPR auto DEFAULT_SERVER_CALC_LATENCY_INTERVAL = 1000;

CONSTEXPR auto SERVERNAME_LENGTH = 128;

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Server)
NET_DSA_BEGIN
NET_CLASS_BEGIN(IPRef)
char* pointer;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(IPRef, PCSTR)
NET_CLASS_DESTRUCTUR(IPRef)

PCSTR get() const;
NET_CLASS_END

NET_ABSTRAC_CLASS_BEGIN(Server, Package)
NET_CLASS_PUBLIC
#pragma region PEERS TABLE

#pragma region Network Structure
NET_STRUCT_BEGIN(network_t)
byte _dataReceive[DEFAULT_SERVER_MAX_PACKET_SIZE];
CPOINTER<byte> _data;
size_t _data_size;
size_t _data_full_size;
size_t _data_offset;

NET_STRUCT_BEGIN_CONSTRUCTUR(network_t)
reset();
clear();
NET_STRUCT_END_CONTRUCTION

void setData(byte*);

void allocData(size_t);
void deallocData();

byte* getData() const;

void reset();
void clear();

void setDataSize(size_t);
size_t getDataSize() const;

void setDataFullSize(size_t);
size_t getDataFullSize() const;

void SetDataOffset(size_t);
size_t getDataOffset() const;

bool dataValid() const;

byte* getDataReceive();
NET_STRUCT_END
#pragma endregion

#pragma region Cryption Structure
NET_STRUCT_BEGIN(cryption_t)
NET_RSA* RSA;
bool RSAHandshake; // set to true as soon as we have the public key from the Peer

NET_STRUCT_BEGIN_CONSTRUCTUR(cryption_t)
RSA = nullptr;
RSAHandshake = false;
NET_STRUCT_END_CONTRUCTION

void createKeyPair(size_t);
void deleteKeyPair();

void setHandshakeStatus(bool);
bool getHandshakeStatus() const;
NET_STRUCT_END
#pragma endregion

// table to keep track of each client's socket
NET_STRUCT_BEGIN(NET_IPEER)
NET_UID UniqueID;
SOCKET pSocket;
struct sockaddr_in client_addr;

bool estabilished;

network_t network;
cryption_t cryption;

/* Async Handler */
bool isAsync;

/* Net Version */
bool NetVersionMatched;

typeLatency latency;
bool bLatency;
NET_HANDLE_TIMER hCalcLatency;

bool bHasBeenErased;
bool bQueueLock;

NET_STRUCT_BEGIN_CONSTRUCTUR(peerInfo)
UniqueID = INVALID_UID;
pSocket = INVALID_SOCKET;
client_addr = sockaddr_in();
estabilished = false;
isAsync = false;
NetVersionMatched = false;
latency = -1;
bLatency = false;
hCalcLatency = nullptr;
bHasBeenErased = false;
bQueueLock = false;
NET_STRUCT_END_CONTRUCTION

void clear();
void setAsync(bool);
typeLatency getLatency() const;
IPRef IPAddr() const;

void lock();
void unlock();
NET_STRUCT_END

NET_CLASS_PRIVATE
void CompressData(BYTE*&, size_t&) const;
void DecompressData(BYTE*&, size_t&) const;

NET_CLASS_PUBLIC
void DisconnectPeer(NET_PEER, int, bool = false);
#pragma endregion

bool ErasePeer(NET_PEER);

NET_CLASS_PRIVATE
size_t _CounterPeersTable;
void IncreasePeersCounter();
void DecreasePeersCounter();
NET_PEER CreatePeer(sockaddr_in, SOCKET);

size_t GetNextPackageSize(NET_PEER) const;
size_t GetReceivedPackageSize(NET_PEER) const;
float GetReceivedPackageSizeAsPerc(NET_PEER) const;

char sServerName[SERVERNAME_LENGTH];
u_short sServerPort;
DWORD sfrequenz;
size_t sRSAKeySize;
size_t sAESKeySize;
bool sCryptPackage;
bool sCompressPackage;
long sTCPReadTimeout;
long sCalcLatencyInterval;
std::vector<Option_t<void*>> option;
std::vector<SocketOption_t<void*>> socketoption;

NET_CLASS_PUBLIC
void SetAllToDefault();
void SetServerName(const char*);
void SetServerPort(u_short);
void SetFrequenz(DWORD);
void SetTimeSpamProtection(float);
void SetRSAKeySize(size_t);
void SetAESKeySize(size_t);
void SetCryptPackage(bool);
void SetCompressPackage(bool);
void SetTCPReadTimeout(long);
void SetCalcLatencyInterval(long);

template <class T>
void SetOption(const Option_t<T> o)
{
	Option_t<void*> opt;
	opt.opt = o.opt;
	opt.type = reinterpret_cast<void*>(o.type);
	opt.len = o.len;
	option.emplace_back(opt);
}

bool Isset(DWORD);

template <class T>
T GetOption(const DWORD opt)
{
	for (const auto& entry : option)
		if (entry.opt == opt)
		{
			return reinterpret_cast<T>(entry.type);
			break;
		}

	return NULL;
}

template <class T>
void SetSocketOption(const SocketOption_t<T> opt)
{
	SocketOption_t<void*> option;
	option.opt = opt.opt;
	option.type = reinterpret_cast<void*>(opt.type);
	option.len = opt.len;
	socketoption.emplace_back(option);
}

const char* GetServerName() const;
u_short GetServerPort() const;
DWORD GetFrequenz() const;
size_t GetRSAKeySize() const;
size_t GetAESKeySize() const;
bool GetCryptPackage() const;
bool GetCompressPackage() const;
long GetTCPReadTimeout() const;
long GetCalcLatencyInterval() const;

bool DoExit;

NET_CLASS_PRIVATE
SOCKET ListenSocket;
SOCKET AcceptSocket;

bool bRunning;

NET_CLASS_PUBLIC
void SetListenSocket(SOCKET);
void SetAcceptSocket(SOCKET);
void SetRunning(bool);

SOCKET GetListenSocket() const;
SOCKET GetAcceptSocket() const;
bool IsRunning() const;

NET_CLASS_CONSTRUCTUR(Server)
NET_CLASS_VDESTRUCTUR(Server)
bool Start(const char*, u_short);
bool Close();

short Handshake(NET_PEER);

NET_CLASS_PUBLIC
NET_DEFINE_CALLBACK(void, Tick) {}
NET_DEFINE_CALLBACK(bool, CheckData, NET_PEER peer, int id, NET_PACKAGE pkg) { return false; }
bool NeedExit() const;

void SingleSend(NET_PEER, const char*, size_t, bool&);
void SingleSend(NET_PEER, BYTE*&, size_t, bool&);
void SingleSend(NET_PEER, CPOINTER<BYTE>&, size_t, bool&);
void DoSend(NET_PEER, int, NET_PACKAGE);

size_t getCountPeers() const;

void Acceptor();
DWORD DoReceive(NET_PEER);

NET_DEFINE_CALLBACK(void, OnPeerUpdate, NET_PEER) {}

NET_CLASS_PRIVATE
void ProcessPackages(NET_PEER);
bool ExecutePackage(NET_PEER);

bool CheckDataN(NET_PEER peer, int id, NET_PACKAGE pkg);

/* Native Packages */
NET_DEF_FNC_PKG(RSAHandshake);
NET_DEF_FNC_PKG(VersionPackage);

NET_CLASS_PROTECTED
/* CALLBACKS */
NET_DEFINE_CALLBACK(void, OnPeerConnect, NET_PEER) {}
NET_DEFINE_CALLBACK(void, OnPeerDisconnect, NET_PEER) {}
NET_DEFINE_CALLBACK(void, OnPeerEstabilished, NET_PEER) {}
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END