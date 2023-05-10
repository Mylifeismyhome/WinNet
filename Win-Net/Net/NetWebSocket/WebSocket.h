/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#define NET_WEB_SOCKET Net::WebSocket::Server

#define NET_PEER Net::WebSocket::Server::peerInfo*

#define PEER peer
#define PKG pkg
#define FUNCTION_NAME NET_FUNCTIONNAME
#define NET_BEGIN_PACKET(cs, fnc) void cs::On##fnc(NET_PEER PEER, NET_PACKET& PKG) { \
	const char* NET_FUNCTIONNAME = CASTRING("On"#fnc);

#define NET_END_PACKET }
#define NET_DECLARE_PACKET(fnc) void On##fnc(NET_PEER, NET_PACKET&)

#define NET_NATIVE_PACKET_DEFINITION_BEGIN(classname) \
bool classname::CheckDataN(NET_PEER peer, const int id, NET_PACKET& pkg) \
{ \
if(!peer) \
	return false; \
switch (id) \
{

#define NET_PACKET_DEFINITION_BEGIN(classname) \
bool classname::CheckData(NET_PEER peer, const int id, NET_PACKET& pkg) \
{ \
if(!peer || !peer->estabilished) \
	return false; \
switch (id) \
{

#define NET_DEFINE_PACKET(xxx, yyy) \
    case yyy: \
    { \
      On##xxx(peer, pkg); \
      break; \
    }

#define NET_PACKET_DEFINITION_END \
	default: \
		return false; \
} \
return true; \
}

#define NET_SEND DoSend

#define SERVERNAME(instance) instance->Isset(NET_OPT_NAME) ? instance->GetOption<char*>(NET_OPT_NAME) : NET_OPT_DEFAULT_NAME
#define SERVERPORT(instance) instance->Isset(NET_OPT_PORT) ? instance->GetOption<u_short>(NET_OPT_PORT) : NET_OPT_DEFAULT_PORT
#define FREQUENZ(instance) instance->Isset(NET_OPT_FREQUENZ) ? instance->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ

#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>
#include <Net/Net/NetCodes.h>
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

/* Websocket frame protocol operationcodes */
CONSTEXPR auto NET_OPCODE_CONTINUE = 0x0;
CONSTEXPR auto NET_OPCODE_TEXT = 0x1;
CONSTEXPR auto NET_OPCODE_BINARY = 0x2;
CONSTEXPR auto NET_OPCODE_CLOSE = 0x8;
CONSTEXPR auto NET_OPCODE_PING = 0x9;
CONSTEXPR auto NET_OPCODE_PONG = 0xA;

CONSTEXPR auto NET_WS_CONTROL_PACKET = -1; // used to send a ping or pong frame
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

#include <openssl/err.h>

#include <Net/Protocol/ICMP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>
#include <Net/assets/manager/filemanager.h>

#include <mutex>

NET_DSA_BEGIN
namespace Net
{
	namespace WebSocket
	{
		class IPRef
		{
			char* pointer;

		public:
			IPRef(const char*);
			~IPRef();

			const char* get() const;
		};

		class Server
		{
			struct network_t
			{
				byte _dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
				NET_CPOINTER<byte> _data;
				size_t _data_size;
				NET_CPOINTER<byte> _dataFragment;
				size_t _data_sizeFragment;
				std::mutex _mutex_send;

				network_t()
				{
					reset();
					clear();
				}

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
			};

		public:
			struct peerInfo
			{
				NET_UID UniqueID;
				SOCKET pSocket;
				struct sockaddr_in client_addr;

				bool estabilished;

				/* network data */
				network_t network;

				/* Erase Handler */
				bool bErase;

				/* SSL */
				SSL* ssl;

				/* Handshake */
				bool handshake;

				typeLatency latency;
				NET_HANDLE_TIMER hCalcLatency;

				std::mutex _mutex_disconnectPeer;

				peerInfo()
				{
					UniqueID = INVALID_UID;
					pSocket = INVALID_SOCKET;
					client_addr = sockaddr_in();
					estabilished = false;
					bErase = false;
					ssl = nullptr;
					handshake = false;
					latency = -1;
					hCalcLatency = nullptr;
				}

				void clear();
				IPRef IPAddr() const;
			};

		private:
			NET_PEER CreatePeer(sockaddr_in, SOCKET);

			DWORD optionBitFlag;
			std::vector<OptionInterface_t*> option;

			DWORD socketOptionBitFlag;
			std::vector<SocketOptionInterface_t*> socketoption;

			SOCKET ListenSocket;
			SOCKET AcceptSocket;

			bool bRunning;
			bool bShuttingDown;

			void DecodeFrame(NET_PEER);
			void EncodeFrame(BYTE*, size_t, NET_PEER, unsigned char = NET_OPCODE_TEXT);
			void ProcessPacket(NET_PEER, BYTE*, size_t);

		private:
			Net::PeerPool::PeerPool_t PeerPoolManager;

		public:
			Server();
			virtual ~Server();

			void DisconnectPeer(NET_PEER, int);

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
				option.emplace_back(ALLOC<Option_t<T>, Option_t<T>>(1, o));

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
				socketoption.emplace_back(ALLOC<SocketOption_t<T>, SocketOption_t<T>>(1, opt));

				// set the bit flag
				socketOptionBitFlag |= opt.opt;
			}

			bool Isset_SocketOpt(DWORD) const;

			void SetListenSocket(SOCKET);
			void SetAcceptSocket(SOCKET);
			void SetRunning(bool);

			SOCKET GetListenSocket() const;
			SOCKET GetAcceptSocket() const;
			bool IsRunning() const;

			bool ErasePeer(NET_PEER, bool = false);

			bool Run();
			bool Close();

			void add_to_peer_threadpool(Net::PeerPool::peerInfo_t);
			void add_to_peer_threadpool(Net::PeerPool::peerInfo_t*);

			size_t count_peers_all();
			size_t count_peers(Net::PeerPool::peer_threadpool_t* pool);
			size_t count_pools();

			SSL_CTX* ctx;
			void onSSLTimeout(NET_PEER);

			bool CheckDataN(NET_PEER peer, int id, NET_PACKET& pkg);

			NET_DEFINE_CALLBACK(void, Tick) {}
			NET_DEFINE_CALLBACK(bool, CheckData, NET_PEER peer, const int id, NET_PACKET& pkg) { return false; }
			void DoSend(NET_PEER, uint32_t, NET_PACKET&, unsigned char = NET_OPCODE_TEXT);
			void DoSend(NET_PEER, uint32_t, BYTE*, size_t, unsigned char = NET_OPCODE_BINARY);

			short Handshake(NET_PEER);
			void Acceptor();
			bool DoReceive(NET_PEER);

			NET_DEFINE_CALLBACK(void, OnPeerEstabilished, NET_PEER) {}
			NET_DEFINE_CALLBACK(void, OnPeerUpdate, NET_PEER) {}

		protected:
			/* CALLBACKS */
			NET_DEFINE_CALLBACK(void, OnPeerConnect, NET_PEER) {}
			NET_DEFINE_CALLBACK(void, OnPeerDisconnect, NET_PEER, int last_error) {}
		};
	}
}
NET_DSA_END