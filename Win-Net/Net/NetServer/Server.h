#pragma once
#define NET_SERVER Net::Server::Server

#define NET_IPEER Net::Server::Server::peerInfo
#define NET_PEER Net::Server::Server::peerInfo*

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

#define SERVERNAME(instance) instance->Isset(NET_OPT_NAME) ? instance->GetOption<SOCKET_OPT_TYPE>(NET_OPT_NAME) : NET_OPT_DEFAULT_NAME
#define SERVERPORT(instance) instance->Isset(NET_OPT_PORT) ? instance->GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT
#define FREQUENZ(instance) instance->Isset(NET_OPT_FREQUENZ) ? instance->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <Net/Net/NetCodes.h>
#include <Net/Net/NetVersion.h>

#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>
#include <Net/Coding/MD5.h>
#include <Net/Coding/BASE64.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/TOTP.h>
#include <Net/Compression/Compression.h>

//#include <Net/Protocol/ICMP.h>
#include <Net/Protocol/NTP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>

#include <Net/Net/NetPeerPool.h>

#ifndef BUILD_LINUX
#pragma warning(disable: 4302)
#pragma warning(disable: 4065)
#endif

#ifdef BUILD_LINUX
#define LAST_ERROR errno
#else
#define LAST_ERROR Ws2_32::WSAGetLastError()
#endif

NET_DSA_BEGIN
namespace Net
{
	namespace Server
	{
		class IPRef
		{
			char* pointer;

		public:
			IPRef(const char*);
			~IPRef();

			const char* get() const;
		};

		class Server : public Net::Package::Package
		{
			struct network_t
			{
				byte _dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
				CPOINTER<byte> _data;
				size_t _data_size;
				size_t _data_full_size;
				size_t _data_offset;
				std::recursive_mutex _mutex_send;

				network_t()
				{
					reset();
					clear();
				}

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
			};

			struct cryption_t
			{
				NET_RSA RSA;
				bool RSAHandshake; // set to true as soon as we have the public key from the Peer

				cryption_t()
				{
					RSAHandshake = false;
				}

				void createKeyPair(size_t);
				void deleteKeyPair();

				void setHandshakeStatus(bool);
				bool getHandshakeStatus() const;
			};

		public:
			struct peerInfo
			{
				NET_UID UniqueID;
				SOCKET pSocket;
				struct sockaddr_in client_addr;

				bool estabilished;

				network_t network;
				cryption_t cryption;

				/* Erase Handler */
				bool bErase;

				/* Net Version */
				bool NetVersionMatched;

				typeLatency latency;
				NET_HANDLE_TIMER hCalcLatency;

				/* TOTP secret */
				byte* totp_secret;
				size_t totp_secret_len;

				/* shift token */
				uint32_t curToken;
				uint32_t lastToken;

				NET_HANDLE_TIMER hWaitForNetProtocol;

				peerInfo()
				{
					UniqueID = INVALID_UID;
					pSocket = INVALID_SOCKET;
					client_addr = sockaddr_in();
					estabilished = false;
					bErase = false;
					NetVersionMatched = false;
					latency = -1;
					hCalcLatency = nullptr;
					totp_secret = nullptr;
					totp_secret_len = NULL;
					curToken = NULL;
					lastToken = NULL;
					hWaitForNetProtocol = nullptr;
				}

				void clear();
				typeLatency getLatency() const;
				IPRef IPAddr() const;
			};

		private:
			Net::PeerPool::PeerPool_t PeerPoolManager;

		public:
			/* time */
			time_t curTime;
			NET_HANDLE_TIMER hSyncClockNTP;
			NET_HANDLE_TIMER hReSyncClockNTP;

		private:
			NET_PEER CreatePeer(sockaddr_in, SOCKET);

			size_t GetNextPackageSize(NET_PEER);
			size_t GetReceivedPackageSize(NET_PEER);
			float GetReceivedPackageSizeAsPerc(NET_PEER);

			DWORD optionBitFlag;
			std::vector<OptionInterface_t*> option;

			DWORD socketOptionBitFlag;
			std::vector<SocketOptionInterface_t*> socketoption;

			SOCKET ListenSocket;

			bool bRunning;

			bool ValidHeader(NET_PEER, bool&);
			void ProcessPackages(NET_PEER);
			void ExecutePackage(NET_PEER);

			bool CheckDataN(NET_PEER peer, int id, NET_PACKAGE pkg);

			/* Native Packages */
			NET_DEF_FNC_PKG(RSAHandshake);
			NET_DEF_FNC_PKG(VersionPackage);

			void CompressData(BYTE*&, size_t&);
			void CompressData(BYTE*&, BYTE*&, size_t&, bool = false);
			void DecompressData(BYTE*&, size_t&);
			void DecompressData(BYTE*&, BYTE*&, size_t&, bool = false);
			bool CreateTOTPSecret(NET_PEER);

		public:
			Server();
			virtual ~Server();

			void DisconnectPeer(NET_PEER, int, bool = false);
			bool ErasePeer(NET_PEER, bool = false);

			template <class T>
			void SetOption(Option_t<T> o)
			{
				// check option is been set using bitflag
				if (optionBitFlag & o.opt)
				{
					// reset the option value
					for (auto& entry : option)
						if (entry->opt == o.opt)
						{
							if (dynamic_cast<Option_t<T>*>(entry))
							{
								dynamic_cast<Option_t<T>*>(entry)->set(o.value());
								return;
							}
						}
				}

				// save the option value
				option.emplace_back(new Option_t<T>(o));

				// set the bit flag
				optionBitFlag |= o.opt;
			}

			bool Isset(DWORD) const;

			template <class T>
			T GetOption(const DWORD opt)
			{
				if (!Isset(opt)) return NULL;
				for (auto& entry : option)
					if (entry->opt == opt)
						if (dynamic_cast<Option_t<T>*>(entry))
							return dynamic_cast<Option_t<T>*>(entry)->value();

				return NULL;
			}

			template <class T>
			void SetSocketOption(SocketOption_t<T> opt)
			{
				// check option is been set using bitflag
				if (socketOptionBitFlag & opt.opt)
				{
					// reset the option value
					for (auto& entry : socketoption)
						if (entry->opt == opt.opt)
						{
							if (dynamic_cast<SocketOption_t<T>*>(entry))
							{
								dynamic_cast<SocketOption_t<T>*>(entry)->set(opt.val());
								return;
							}
						}
				}

				// save the option value
				socketoption.emplace_back(new SocketOption_t<T>(opt));

				// set the bit flag
				socketOptionBitFlag |= opt.opt;
			}

			bool Isset_SocketOpt(DWORD) const;

			void SetListenSocket(SOCKET);
			void SetRunning(bool);

			SOCKET GetListenSocket() const;
			bool IsRunning() const;

			bool Run();
			bool Close();

			NET_DEFINE_CALLBACK(void, Tick) {}
			NET_DEFINE_CALLBACK(bool, CheckData, NET_PEER peer, int id, NET_PACKAGE pkg) { return false; }
			void SingleSend(NET_PEER, const char*, size_t, bool&, uint32_t = INVALID_UINT_SIZE);
			void SingleSend(NET_PEER, BYTE*&, size_t, bool&, uint32_t = INVALID_UINT_SIZE);
			void SingleSend(NET_PEER, CPOINTER<BYTE>&, size_t, bool&, uint32_t = INVALID_UINT_SIZE);
			void SingleSend(NET_PEER, Net::Package::Package_RawData_t&, bool&, uint32_t = INVALID_UINT_SIZE);
			void DoSend(NET_PEER, int, NET_PACKAGE);

			void add_to_peer_threadpool(Net::PeerPool::peerInfo_t);
			void add_to_peer_threadpool(Net::PeerPool::peerInfo_t*);

			size_t count_peers_all();
			size_t count_peers(Net::PeerPool::peer_threadpool_t* pool);
			size_t count_pools();

			void Acceptor();
			bool DoReceive(NET_PEER);

			NET_DEFINE_CALLBACK(void, OnPeerUpdate, NET_PEER) {}

		protected:
			NET_DEFINE_CALLBACK(void, OnPeerConnect, NET_PEER) {}
			NET_DEFINE_CALLBACK(void, OnPeerDisconnect, NET_PEER, int last_error) {}
			NET_DEFINE_CALLBACK(void, OnPeerEstabilished, NET_PEER) {}
		};
	}
}
NET_DSA_END