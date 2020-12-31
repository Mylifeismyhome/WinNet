#pragma once
#define NET_WEB_SERVER Net::WebServer::Server

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

#define SERVERNAME(instance) instance->Isset(NET_OPT_NAME) ? instance->GetOption<char*>(NET_OPT_NAME) : NET_OPT_DEFAULT_NAME
#define SERVERPORT(instance) instance->Isset(NET_OPT_PORT) ? instance->GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT
#define FREQUENZ(instance) instance->Isset(NET_OPT_FREQUENZ) ? instance->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <Net/Net/NetCodes.h>

/* Websocket frame protocol operationcodes */
CONSTEXPR auto NET_OPCODE_CONTINUE = 0x0;
CONSTEXPR auto NET_OPCODE_TEXT = 0x1;
CONSTEXPR auto NET_OPCODE_BINARY = 0x2;
CONSTEXPR auto NET_OPCODE_CLOSE = 0x8;
CONSTEXPR auto NET_OPCODE_PING = 0x9;
CONSTEXPR auto NET_OPCODE_PONG = 0xA;

CONSTEXPR auto NET_WS_CONTROL_PACKAGE = -1; // used to send a ping or pong frame
CONSTEXPR auto NET_WS_FIN = 0x80;
CONSTEXPR auto NET_WS_OPCODE = 0xF;
CONSTEXPR auto NET_WS_MASK = 0x7F;
CONSTEXPR auto NET_WS_PAYLOADLENGTH = 0x7F;
CONSTEXPR auto NET_WS_CONTROLFRAME = 8;
CONSTEXPR auto NET_WS_PAYLOAD_LENGTH_16 = 126;
CONSTEXPR auto NET_WS_PAYLOAD_LENGTH_63 = 127;

#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>
#include <Net/Coding/MD5.h>
#include <Net/Coding/BASE64.h>
#include <Net/Coding/SHA1.h>
#include <Net/Compression/Compression.h>

#include <OpenSSL/ssl.h>
#include <OpenSSL/err.h>

#include <Net/Protocol/ICMP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(WebServer)
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
byte _dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
CPOINTER<byte> _data;
size_t _data_size;
CPOINTER<byte> _dataFragment;
size_t _data_sizeFragment;

NET_STRUCT_BEGIN_CONSTRUCTUR(network_t)
reset();
clear();
NET_STRUCT_END_CONTRUCTION

void setData(byte*);
void setDataFragmented(byte*);

void allocData(size_t);
void deallocData();

void allocDataFragmented(size_t);
void deallocDataFragmented();

byte* getData() const;
byte* getDataFragmented() const;

void reset();
void clear();

void setDataSize(size_t);
size_t getDataSize() const;

void setDataFragmentSize(size_t);
size_t getDataFragmentSize() const;

bool dataValid() const;
bool dataFragmentValid() const;

byte* getDataReceive();
NET_STRUCT_END
#pragma endregion

// table to keep track of each client's socket
NET_STRUCT_BEGIN(NET_IPEER)
NET_UID UniqueID;
SOCKET pSocket;
struct sockaddr_in client_addr;
float lastaction;

bool estabilished;

/* network data */
network_t network;

/* Async Handler */
bool isAsync;

/* SSL */
SSL* ssl;

/* Handshake */
bool handshake;

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
ssl = nullptr;
handshake = false;
latency = -1;
bLatency = false;
hCalcLatency = nullptr;
bHasBeenErased = false;
bQueueLock = false;
NET_STRUCT_END_CONTRUCTION

void clear();
void setAsync(bool);
IPRef IPAddr() const;

void lock();
void unlock();
NET_STRUCT_END

void DisconnectPeer(NET_PEER, int);
#pragma endregion

NET_CLASS_PRIVATE
size_t _CounterPeersTable;
void IncreasePeersCounter();
void DecreasePeersCounter();
NET_PEER CreatePeer(sockaddr_in, SOCKET);

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

NET_CLASS_PRIVATE
SOCKET ListenSocket;
SOCKET AcceptSocket;

bool bRunning;
bool bShuttingDown;

NET_CLASS_PUBLIC
void SetListenSocket(SOCKET);
void SetAcceptSocket(SOCKET);
void SetRunning(bool);

SOCKET GetListenSocket() const;
SOCKET GetAcceptSocket() const;
bool IsRunning() const;

bool ErasePeer(NET_PEER);

NET_CLASS_CONSTRUCTUR(Server)
NET_CLASS_VDESTRUCTUR(Server)
bool Run();
bool Close();

bool DoExit;

short Handshake(NET_PEER);
void Acceptor();

SSL_CTX* ctx;
void onSSLTimeout(NET_PEER);

bool CheckDataN(NET_PEER peer, int id, NET_PACKAGE pkg);

NET_CLASS_PUBLIC
NET_DEFINE_CALLBACK(void, Tick) {}
NET_DEFINE_CALLBACK(bool, CheckData, NET_PEER peer, const int id, NET_PACKAGE pkg) { return false; }
bool NeedExit() const;

void DoSend(NET_PEER, int, NET_PACKAGE, unsigned char = NET_OPCODE_TEXT);

size_t getCountPeers() const;

DWORD DoReceive(NET_PEER);

/* CALLBACKS */
NET_DEFINE_CALLBACK(void, OnPeerUpdate, NET_PEER) {}
NET_DEFINE_CALLBACK(void, OnPeerEstabilished, NET_PEER) {}

NET_CLASS_PRIVATE
void DecodeFrame(NET_PEER);
void EncodeFrame(const char*, size_t, NET_PEER, unsigned char = NET_OPCODE_TEXT);
bool ProcessPackage(NET_PEER, BYTE*, size_t);

NET_CLASS_PROTECTED
/* CALLBACKS */
NET_DEFINE_CALLBACK(void, OnPeerConnect, NET_PEER) {}
NET_DEFINE_CALLBACK(void, OnPeerDisconnect, NET_PEER) {}
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END