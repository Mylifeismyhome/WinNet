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

#define FREQUENZ Isset(NET_OPT_FREQUENZ) ? GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <Net/Net/NetCodes.h>
#include <Net/Net/NetVersion.h>

#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>
#include <Net/Compression/Compression.h>
#include <Net/Cryption/PointerCryption.h>
#include <Net/Coding/SHA1.h>

#include <Net/Protocol/ICMP.h>
#include <Net/Protocol/NTP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Client)
NET_DSA_BEGIN
NET_ABSTRAC_CLASS_BEGIN(Client, Package)
NET_STRUCT_BEGIN(Network)
byte dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
CPOINTER<byte> data;
size_t data_size;
size_t data_full_size;
size_t data_offset;
bool recordingData;
NET_RSA* RSA;
bool RSAHandshake; // set to true as soon as we have the public key from the Server

bool estabilished;

typeLatency latency;
bool bLatency;
NET_HANDLE_TIMER hCalcLatency;

NET_STRUCT_BEGIN_CONSTRUCTUR(Network)
memset(dataReceive, NULL, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
data = nullptr;
data_size = 0;
data_full_size = 0;
data_offset = 0;
recordingData = false;
RSA = nullptr;
RSAHandshake = false;
estabilished = false;
latency = -1;
bLatency = false;
hCalcLatency = nullptr;
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
bool NeedExit;
bool bAccomplished;
SOCKET connectSocket;
struct addrinfo* connectSocketAddr;
CPOINTER<char> ServerAddress;
u_short ServerPort;
bool connected;

void SetRecordingData(bool);

void Timeout();

/* clear all stored data */
void ConnectionClosed();

void CompressData(BYTE*&, size_t&);
void DecompressData(BYTE*&, size_t&);

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(Client)
NET_CLASS_VDESTRUCTUR(Client)

bool ChangeMode(bool);

char* ResolveHostname(const char*);
bool Connect(const char*, u_short);
bool Disconnect();
void Clear();
bool DoNeedExit() const;
void WaitUntilAccomplished();

void SetSocket(const SOCKET);
SOCKET GetSocket() const;

void SetServerAddress(const char*);
const char* GetServerAddress() const;

void SetServerPort(u_short);
u_short GetServerPort() const;

void SetConnected(bool);
bool IsConnected() const;

size_t GetNextPackageSize() const;
size_t GetReceivedPackageSize() const;
float GetReceivedPackageSizeAsPerc() const;

DWORD DoReceive();
bool CheckDataN(int id, NET_PACKAGE pkg);
NET_DEFINE_CALLBACK(bool, CheckData, const int id, NET_PACKAGE pkg) { return false; }

void SingleSend(const char*, size_t, bool&);
void SingleSend(BYTE*&, size_t, bool&);
void SingleSend(CPOINTER<BYTE>&, size_t, bool&);
void DoSend(int, NET_PACKAGE);

NET_CLASS_PRIVATE
void ProcessPackages();
void ExecutePackage();
bool BuildNTPHash();

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