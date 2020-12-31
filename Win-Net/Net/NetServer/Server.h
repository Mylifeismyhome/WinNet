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

#define SERVERNAME(instance) instance->Isset(OPT_ServerName) ? instance->GetOption<char*>(OPT_ServerName) : DEFAULT_OPTION_SERVERNAME
#define SERVERPORT(instance) instance->Isset(OPT_ServerPort) ? instance->GetOption<u_short>(OPT_ServerPort) : DEFAULT_OPTION_SERVERPORT
#define FREQUENZ(instance) instance->Isset(OPT_Frequenz) ? instance->GetOption<DWORD>(OPT_Frequenz) : DEFAULT_OPTION_FREQUENZ

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

CONSTEXPR size_t DEFAULT_MAX_PACKET_SIZE = 512;

CONSTEXPR auto DEFAULT_OPTION_SERVERNAME = "UNKNOWN";
CONSTEXPR auto DEFAULT_OPTION_SERVERPORT = 50000;
CONSTEXPR size_t DEFAULT_OPTION_RSA_KEY_SIZE = 1024;
CONSTEXPR auto DEFAULT_OPTION_AES_KEY_SIZE = CryptoPP::AES::MAX_KEYLENGTH;
CONSTEXPR auto DEFAULT_OPTION_CRYPT_PACKAGES = false;
CONSTEXPR auto DEFAULT_OPTION_COMPRESS_PACKAGES = false;
CONSTEXPR auto DEFAULT_OPTION_FREQUENZ = 66;
CONSTEXPR auto DEFAULT_OPTION_CALC_LATENCY_INTERVAL = 1000;
CONSTEXPR auto DEFAULT_OPTION_TCP_READ_TIMEOUT = 10;

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
byte _dataReceive[DEFAULT_MAX_PACKET_SIZE];
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
void CompressData(BYTE*&, size_t&);
void DecompressData(BYTE*&, size_t&);

NET_CLASS_PUBLIC
void DisconnectPeer(NET_PEER, int, bool = false);
#pragma endregion

bool ErasePeer(NET_PEER);

NET_CLASS_PRIVATE
size_t _CounterPeersTable;
void IncreasePeersCounter();
void DecreasePeersCounter();
NET_PEER CreatePeer(sockaddr_in, SOCKET);

size_t GetNextPackageSize(NET_PEER);
size_t GetReceivedPackageSize(NET_PEER);
float GetReceivedPackageSizeAsPerc(NET_PEER);

DWORD optionBitFlag;
std::vector<Option_t<char*>> option;

DWORD socketOptionBitFlag;
std::vector<SocketOption_t<char*>> socketoption;

NET_CLASS_PUBLIC
template <class T>
void SetOption(const Option_t<T> o)
{
	// check option is been set using bitflag
	if (optionBitFlag & o.opt)
	{
		// reset the option value
		for (auto& entry : option)
			if (entry.opt == o.opt)
			{
				entry.type = reinterpret_cast<char*>(o.type);
				entry.len = o.len;
				return;
			}
	}

	// save the option value
	Option_t<char*> opt;
	opt.opt = o.opt;
	opt.type = reinterpret_cast<char*>(o.type);
	opt.len = o.len;
	option.emplace_back(opt);

	// set the bit flag
	optionBitFlag |= opt.opt;
}

bool Isset(DWORD) const;

template <class T>
T GetOption(const DWORD opt)
{
	if (!Isset(opt)) return NULL;
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
	// check option is been set using bitflag
	if (socketOptionBitFlag & opt.opt)
	{
		// reset the option value
		for (auto& entry : socketoption)
			if (entry.opt == opt.opt)
			{
				entry.type = reinterpret_cast<char*>(opt.type);
				entry.len = opt.len;
				return;
			}
	}

	// save the option value
	SocketOption_t<char*> option;
	option.opt = opt.opt;
	option.type = reinterpret_cast<char*>(opt.type);
	option.len = opt.len;
	socketoption.emplace_back(option);

	// set the bit flag
	socketOptionBitFlag |= option.opt;
}

bool Isset_SocketOpt(DWORD) const;

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
bool Run();
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
void ExecutePackage(NET_PEER);

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