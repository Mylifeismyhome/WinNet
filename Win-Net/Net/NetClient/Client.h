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
#define NET_CLIENT Net::Client::Client

#define PKG pkg
#define FUNCTION_NAME NET_FUNCTIONNAME
#define NET_BEGIN_PACKET(cs, fnc) void cs::On##fnc(NET_PACKET& PKG) { \
	const char* NET_FUNCTIONNAME = CASTRING("On"#fnc);

#define NET_END_PACKET }
#define NET_DECLARE_PACKET(fnc) void On##fnc(NET_PACKET&)

#define NET_NATIVE_PACKET_DEFINITION_BEGIN(classname) \
bool classname::CheckDataN(const int id, NET_PACKET& pkg) \
{ \
switch (id) \
{

#define NET_PACKET_DEFINITION_BEGIN(classname) \
bool classname::CheckData(const int id, NET_PACKET& pkg) \
{ \
switch (id) \
{

#define NET_DEFINE_PACKET(xxx, yyy) \
    case yyy: \
    { \
      On##xxx(pkg); \
      break; \
    } \

#define NET_PACKET_DEFINITION_END \
	default: \
		return false; \
} \
return true; \
}

#define NET_SEND DoSend

#define FREQUENZ(x) x->Isset(NET_OPT_FREQUENZ) ? x->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ

#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>
#include <Net/Net/NetCodes.h>
#include <Net/Net/NetVersion.h>

#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>
#include <Net/Compression/Compression.h>
#include <Net/Cryption/PointerCryption.h>

//#include <Net/Protocol/ICMP.h>
#include <Net/Protocol/NTP.h>

#include <Net/assets/thread.h>
#include <Net/assets/timer.h>

#ifndef BUILD_LINUX
#pragma warning(disable: 4302)
#pragma warning(disable: 4065)
#endif

#ifdef BUILD_LINUX
#define LAST_ERROR errno
#define NET_NO_ERROR -1
#else
#define LAST_ERROR Ws2_32::WSAGetLastError()
#define NET_NO_ERROR NO_ERROR
#endif

NET_DSA_BEGIN
namespace Net
{
	namespace Client
	{
		enum ECONNECTION_STATUS
		{
			EDISCONNECTED = 0,
			ECONNECTED,
			EPENDING_DISCONNECT
		};

		class Client
		{
			struct Network
			{
				NET_CPOINTER<byte> dataReceive;
				size_t data_receive_size;
				NET_CPOINTER<byte> data;
				size_t data_size;
				size_t data_full_size;
				size_t data_offset;
				size_t data_original_uncompressed_size;
				bool recordingData;
				NET_RSA RSA;
				bool RSAHandshake; // set to true as soon as we have the public key from the Server

				bool estabilished;

				std::mutex _mutex_send;

				int m_heartbeat_sequence_number;

				Network()
				{
					dataReceive = nullptr;
					data_receive_size = 0;
					data = nullptr;
					data_size = 0;
					data_full_size = 0;
					data_offset = 0;
					data_original_uncompressed_size = 0;
					recordingData = false;
					RSAHandshake = false;
					estabilished = false;
					m_heartbeat_sequence_number = -1;
				}

				void clear();
				void AllocReceiveBuffer(size_t size = NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE);
				void ClearReceiveBuffer();
				void ResetReceiveBuffer();
				void AllocData(size_t);
				void clearData();
				void createNewRSAKeys(size_t);
				void deleteRSAKeys();
			};

		public:
			Network network;

		private:
			DWORD optionBitFlag;
			std::vector<OptionInterface_t*> option;

			DWORD socketOptionBitFlag;
			std::vector<SocketOptionInterface_t*> socketoption;

		public:
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

		private:
			SOCKET connectSocket;
			struct addrinfo* connectSocketAddr;
			NET_CPOINTER<char> ServerAddress;
			u_short ServerPort;
			ECONNECTION_STATUS m_connectionStatus;

			void SetRecordingData(bool);

			void CompressData(BYTE*&, size_t&);
			void CompressData(BYTE*&, BYTE*&, size_t&, bool = false);
			void DecompressData(BYTE*&, size_t&, size_t);
			void DecompressData(BYTE*&, BYTE*&, size_t&, size_t, bool = false);

			NET_THREAD_HANDLE hReceiveThread;

		public:
			Client();
			~Client();

			char* ResolveHostname(const char*);
			bool Connect(const char*, u_short);
			void Disconnect();
			void ConnectionClosed();
			void Clear();

			void SetSocket(const SOCKET);
			SOCKET GetSocket() const;

			void SetServerAddress(const char*);
			const char* GetServerAddress() const;

			void SetServerPort(u_short);
			u_short GetServerPort() const;

		private:
			void SetConnectionStatus(ECONNECTION_STATUS);

		public:
			ECONNECTION_STATUS getConnectionStatus() const;

			size_t GetNextPacketSize() const;
			size_t GetReceivedPacketSize() const;
			float GetReceivedPacketSizeAsPerc() const;

			DWORD DoReceive();

		public:
			bool CheckDataN(int id, NET_PACKET& pkg);
			NET_DEFINE_CALLBACK(bool, CheckData, const int id, NET_PACKET& pkg) { return false; }

		private:
			void SingleSend(const char*, size_t, bool&);
			void SingleSend(BYTE*&, size_t, bool&);
			void SingleSend(NET_CPOINTER<BYTE>&, size_t, bool&);
			void SingleSend(Net::RawData_t&, bool&);

		public:
			void DoSend(int, NET_PACKET&);

		private:
			DWORD ProcessPackets();
			void ExecutePacket();

			NET_DECLARE_PACKET(NetProtocolHandshake);
			NET_DECLARE_PACKET(RSAHandshake);
			NET_DECLARE_PACKET(Keys);
			NET_DECLARE_PACKET(Version);
			NET_DECLARE_PACKET(EstabilishConnection);
			NET_DECLARE_PACKET(Close);
			NET_DECLARE_PACKET(NetHeartbeat);

		protected:
			NET_DEFINE_CALLBACK(void, OnConnected) {}
			NET_DEFINE_CALLBACK(void, OnDisconnected) {}
			NET_DEFINE_CALLBACK(void, OnConnectionClosed, int code) {}
			NET_DEFINE_CALLBACK(void, OnKeysReceived) {}
			NET_DEFINE_CALLBACK(void, OnKeysFailed) {}
			NET_DEFINE_CALLBACK(void, OnConnectionEstabilished) {}
			NET_DEFINE_CALLBACK(void, OnVersionMismatch) {}
		};
	}
}
NET_DSA_END