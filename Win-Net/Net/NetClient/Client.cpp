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

#include <NetClient/Client.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/Import/Ws2_32.hpp>

namespace Net
{
	namespace Client
	{
		static bool AddrIsV4(const char* addr)
		{
			struct sockaddr_in sa;
			if (inet_pton(AF_INET, addr, &(sa.sin_addr)))
				return true;

			return false;
		}

		static bool AddrIsV6(const char* addr)
		{
			struct sockaddr_in6 sa;
			if (inet_pton(AF_INET6, addr, &(sa.sin6_addr)))
				return true;

			return false;
		}

		Client::Client()
		{
			SetSocket(INVALID_SOCKET);
			SetServerAddress(CSTRING(""));
			SetServerPort(0);
			SetConnectionStatus(EDISCONNECTED);
			optionBitFlag = 0;
			socketOptionBitFlag = 0;
			hReceiveThread = 0;
		}

		Client::~Client()
		{
			Clear();

			for (auto& entry : socketoption)
				FREE<SocketOptionInterface_t>(entry);

			socketoption.clear();

			for (auto& entry : option)
				FREE<OptionInterface_t>(entry);

			option.clear();
		}

		NET_THREAD(Receive)
		{
			const auto client = (Client*)parameter;
			if (client == nullptr)
			{
				return 0;
			}

			while (client->getConnectionStatus() == Net::Client::ECONNECTED)
			{
				if (client->GetSocket() == INVALID_SOCKET)
				{
					break;
				}

				const auto ret = client->DoReceive();
				if (ret == -1)
				{
					client->Disconnect();
					break;
				}

#ifdef BUILD_LINUX
				usleep(ret * 1000);
#else
				Kernel32::Sleep(ret);
#endif
			}

			client->ConnectionClosed();
			return 0;
		}

		bool Client::Isset(const DWORD opt) const
		{
			// use the bit flag to perform faster checks
			return optionBitFlag & opt;
		}

		bool Client::Isset_SocketOpt(const DWORD opt) const
		{
			// use the bit flag to perform faster checks
			return socketOptionBitFlag & opt;
		}

		char* Client::ResolveHostname(const char* name)
		{
#ifndef BUILD_LINUX
			WSADATA wsaData;
			auto res = Ws2_32::WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (res != 0)
			{
				NET_LOG_ERROR(CSTRING("[NET] - WSAStartup has been failed with error: %d"), res);
				return nullptr;
			}

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Could not find a usable version of Winsock.dll"));
				Ws2_32::WSACleanup();
				return nullptr;
			}
#endif

			struct addrinfo hints = {};
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			struct addrinfo* result = nullptr;
			const auto dwRetval = Ws2_32::getaddrinfo(name, nullptr, &hints, &result);
			if (dwRetval != 0)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Host look up has been failed with error %d"), dwRetval);
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return nullptr;
			}

			struct sockaddr_in* psockaddrv4 = nullptr;
			struct sockaddr_in6* psockaddrv6 = nullptr;
			struct addrinfo* ptr = nullptr;
			for (ptr = result; ptr != 0; ptr = ptr->ai_next)
			{
				bool v6 = false;
				switch (ptr->ai_family)
				{
				case AF_INET:
					break;

				case AF_INET6:
					v6 = true;
					break;

				default:
					// skip
					continue;
				}

				switch (ptr->ai_socktype)
				{
				case SOCK_STREAM:
					break;

				default:
					// skip
					continue;
				}

				switch (ptr->ai_protocol)
				{
				case IPPROTO_TCP:
					break;

				default:
					// skip
					continue;
				}

				if (v6) psockaddrv6 = (struct sockaddr_in6*)ptr->ai_addr;
				else psockaddrv4 = (struct sockaddr_in*)ptr->ai_addr;

				// break out, we have a connectivity we can use
				break;
			}

			if (!psockaddrv4 && !psockaddrv6)
			{
				Ws2_32::freeaddrinfo(result);
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return nullptr;
			}

			const auto len = psockaddrv6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
			auto buf = ALLOC<char>(len);
			memset(buf, 0, len);

			if (psockaddrv6) buf = (char*)Ws2_32::inet_ntop(psockaddrv6->sin6_family, &psockaddrv6->sin6_addr, buf, INET6_ADDRSTRLEN);
			else buf = (char*)Ws2_32::inet_ntop(psockaddrv4->sin_family, &psockaddrv4->sin_addr, buf, INET_ADDRSTRLEN);

			Ws2_32::freeaddrinfo(result);
#ifndef BUILD_LINUX
			Ws2_32::WSACleanup();
#endif

			return buf;
		}

		bool Client::Connect(const char* Address, const u_short Port)
		{
			if (getConnectionStatus() != EDISCONNECTED)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Can't connect to server, reason: already connected!"));
				return false;
			}

			int res = 0;

#ifndef BUILD_LINUX
			WSADATA wsaData;
			res = Ws2_32::WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (res != 0)
			{
				NET_LOG_ERROR(CSTRING("[NET] - WSAStartup has been failed with error: %d"), res);
				return false;
			}

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Could not find a usable version of Winsock.dll"));
				Ws2_32::WSACleanup();
				return false;
			}
#endif

			SetServerAddress(Address);
			SetServerPort(Port);

			auto v6 = AddrIsV6(GetServerAddress());
			auto v4 = AddrIsV4(GetServerAddress());
			if (!v6 && !v4)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Address is neather IPV4 nor IPV6 Protocol"));
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			SetSocket(Ws2_32::socket(v6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP));
			if (GetSocket() == SOCKET_ERROR)
			{
				NET_LOG_ERROR(CSTRING("[NET] - Unable to create socket, error code: %d"), LAST_ERROR);
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			struct sockaddr* sockaddr = nullptr;
			int slen = 0;
			if (v4)
			{
				struct sockaddr_in sockaddr4;
				memset((char*)&sockaddr4, 0, sizeof(sockaddr4));
				sockaddr4.sin_family = AF_INET;
				sockaddr4.sin_port = htons(GetServerPort());
#ifdef BUILD_LINUX
				sockaddr4.sin_addr.s_addr = inet_addr(GetServerAddress());
#else
				sockaddr4.sin_addr.S_un.S_addr = inet_addr(GetServerAddress());
#endif
				sockaddr = (struct sockaddr*)&sockaddr4;
				slen = static_cast<int>(sizeof(struct sockaddr_in));
			}

			if (v6)
			{
				struct sockaddr_in6 sockaddr6;
				memset((char*)&sockaddr6, 0, sizeof(sockaddr6));
				sockaddr6.sin6_family = AF_INET6;
				sockaddr6.sin6_port = htons(GetServerPort());
				res = inet_pton(AF_INET6, GetServerAddress(), &sockaddr6.sin6_addr);
				if (res != 1)
				{
					NET_LOG_ERROR(CSTRING("[NET]  - Failure on setting IPV6 Address with error code %d"), res);

					// close endpoint
					SOCKET_VALID(GetSocket())
					{
						bool bBlocked = false;
						do
						{
							if (Ws2_32::closesocket(GetSocket()) == SOCKET_ERROR)
							{
#ifdef BUILD_LINUX
								if (errno == EWOULDBLOCK)
#else
								if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
#endif
								{
									bBlocked = true;
									continue;
								}
							}

						} while (bBlocked);

						SetSocket(INVALID_SOCKET);
					}

#ifndef BUILD_LINUX
					Ws2_32::WSACleanup();
#endif
					return false;
				}
				sockaddr = (struct sockaddr*)&sockaddr6;
				slen = static_cast<int>(sizeof(struct sockaddr_in6));
			}

			if (!sockaddr)
			{
				NET_LOG_ERROR(CSTRING("[NET]  - Socket is not being valid"));

				// close endpoint
				SOCKET_VALID(GetSocket())
				{
					bool bBlocked = false;
					do
					{
						bBlocked = false;
						if (Ws2_32::closesocket(GetSocket()) == SOCKET_ERROR)
						{
#ifdef BUILD_LINUX
							if (errno == EWOULDBLOCK)
#else
							if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
#endif
							{
								bBlocked = true;
								continue;
							}
						}

					} while (bBlocked);

					SetSocket(INVALID_SOCKET);
				}

#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			if (GetSocket() == INVALID_SOCKET)
			{
				NET_LOG_ERROR(CSTRING("WinNet :: Client =>  socket failed with error: %ld"), LAST_ERROR);

#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			/* Connect to the server */
			if (Ws2_32::connect(GetSocket(), sockaddr, slen) == SOCKET_ERROR)
			{
				SetSocket(INVALID_SOCKET);

				NET_LOG_ERROR(CSTRING("WinNet :: Client =>  failure on connecting to host: %s:%hu"), GetServerAddress(), GetServerPort());
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			// clear the unused vector
			socketoption.clear();

			if (Net::SetDefaultSocketOption(GetSocket(), (Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE)) == 0)
			{
				NET_LOG_ERROR(CSTRING("WinNet :: Client => failed to apply default socket option for '%d'\n\tdiscarding socket..."), GetSocket());

				SetSocket(INVALID_SOCKET);

#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif

				return false;
			}

			/*
			* Set/override socket options
			*/
			for (const auto& entry : socketoption)
			{
				const auto res = Ws2_32::setsockopt(GetSocket(), entry->level, entry->opt, entry->value(), entry->optlen());
				if (res == SOCKET_ERROR)
				{
					NET_LOG_ERROR(CSTRING("WinNet :: Client =>  failed to apply socket option { %i : %i } for socket '%d'"), entry->opt, LAST_ERROR, GetSocket());
				}
			}

			// successfully connected
			SetConnectionStatus(ECONNECTED);

			network.AllocReceiveBuffer((Isset(NET_OPT_RECEIVE_BUFFER_SIZE) ? GetOption<size_t>(NET_OPT_RECEIVE_BUFFER_SIZE) : NET_OPT_DEFAULT_RECEIVE_BUFFER_SIZE));

			// Create Loop-Receive Thread
			hReceiveThread = Net::Thread::Create(Receive, this);

			// callback
			OnConnected();

			return true;
		}

		void Client::Disconnect()
		{
			if (getConnectionStatus() == EDISCONNECTED)
			{
				return;
			}

			SetConnectionStatus(EPENDING_DISCONNECT);
		}

		void Client::ConnectionClosed()
		{
			// close endpoint
			SOCKET_VALID(GetSocket())
			{
				bool bBlocked = false;
				do
				{
					Ws2_32::shutdown(GetSocket(), SOCKET_WR);
					if (Ws2_32::closesocket(GetSocket()) == SOCKET_ERROR)
					{
#ifdef BUILD_LINUX
						if (errno == EWOULDBLOCK)
#else
						if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
#endif
						{
							bBlocked = true;
							continue;
						}
					}

				} while (bBlocked);

				SetSocket(INVALID_SOCKET);
			}

			if (hReceiveThread)
			{
				Net::Thread::Close(hReceiveThread);
				hReceiveThread = 0;
			}

			Clear();

			// callback
			OnDisconnected();

			// finally set disconnected status
			SetConnectionStatus(EDISCONNECTED);
		}

		void Client::Clear()
		{
#ifndef BUILD_LINUX
			Ws2_32::WSACleanup();
#endif
			network.clear();
		}

		void Client::SetSocket(const SOCKET connectSocket)
		{
			this->connectSocket = connectSocket;
		}

		SOCKET Client::GetSocket() const
		{
			return connectSocket;
		}

		void Client::SetServerAddress(const char* ServerAddress)
		{
			this->ServerAddress.free();
			const auto size = strlen(ServerAddress);
			this->ServerAddress = ALLOC<char>(size + 1);
			memcpy(this->ServerAddress.get(), ServerAddress, size);
			this->ServerAddress.get()[size] = '\0';
		}

		const char* Client::GetServerAddress() const
		{
			return ServerAddress.get();
		}

		void Client::SetServerPort(const u_short ServerPort)
		{
			this->ServerPort = ServerPort;
		}

		u_short Client::GetServerPort() const
		{
			return ServerPort;
		}

		void Client::SetConnectionStatus(ECONNECTION_STATUS status)
		{
			m_connectionStatus = status;
		}

		ECONNECTION_STATUS Client::getConnectionStatus() const
		{
			return m_connectionStatus;
		}

		size_t Client::GetNextPacketSize() const
		{
			return network.data_full_size;
		}

		size_t Client::GetReceivedPacketSize() const
		{
			return network.data_size;
		}

		float Client::GetReceivedPacketSizeAsPerc() const
		{
			// Avoid dividing zero with zero
			if (network.data_size <= 0)
				return 0.0f;

			if (network.data_full_size <= 0)
				return 0.0f;

			auto perc = static_cast<float>(network.data_size) / static_cast<float>(network.data_full_size) * 100;
			if (perc < 0.0f) // should never happen, just in case
				perc = 0.0f;
			else if (perc > 100.0f) // could happen, as we play around with the sizes later on
				perc = 100.0f;

			return perc;
		}

		void Client::Network::clear()
		{
			recordingData = false;
			estabilished = false;
			clearData();
			deleteRSAKeys();
			ClearReceiveBuffer();
		}

		void Client::Network::AllocReceiveBuffer(size_t size)
		{
			if (dataReceive.valid())
			{
				ClearReceiveBuffer();
			}

			dataReceive = ALLOC<byte>(size + 1);
			memset(dataReceive.get(), 0, size);
			dataReceive.get()[size] = 0;

			data_receive_size = size;
		}

		void Client::Network::ClearReceiveBuffer()
		{
			if (dataReceive.valid() == 0)
			{
				return;
			}

			dataReceive.free();
			data_receive_size = 0;
		}

		void Client::Network::ResetReceiveBuffer()
		{
			if (dataReceive.valid() == 0)
			{
				return;
			}

			memset(dataReceive.get(), 0, data_receive_size);
		}

		void Client::Network::AllocData(const size_t size)
		{
			data = ALLOC<byte>(size + 1);
			memset(data.get(), 0, size);
			data_size = 0;
			data_full_size = 0;
			data_offset = 0;
		}

		void Client::Network::clearData()
		{
			data.free();
			data_size = 0;
			data_full_size = 0;
			data_offset = 0;
			data_original_uncompressed_size = 0;
		}

		void Client::SetRecordingData(const bool status)
		{
			network.recordingData = status;
		}

		void Client::Network::createNewRSAKeys(const size_t keySize)
		{
			RSA.generateKeys(keySize, 3);
			RSAHandshake = false;
		}

		void Client::Network::deleteRSAKeys()
		{
			RSA.deleteKeys();
			RSAHandshake = false;
		}

		void Client::SingleSend(const char* data, size_t size, bool& bPreviousSentFailed)
		{
			if (GetSocket() == false)
			{
				return;
			}

			if (bPreviousSentFailed)
			{
				return;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}

				if (res < 0)
				{
					break;
				}

				size -= res;
			} while (size > 0);
		}

		void Client::SingleSend(BYTE*& data, size_t size, bool& bPreviousSentFailed)
		{
			if (GetSocket() == false)
			{
				return;
			}

			if (data == nullptr)
			{
				return;
			}

			if (bPreviousSentFailed)
			{
				return;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}

				if (res < 0)
				{
					break;
				}

				size -= res;
			} while (size > 0);
		}

		void Client::SingleSend(NET_CPOINTER<BYTE>& data, size_t size, bool& bPreviousSentFailed)
		{
			if (GetSocket() == false)
			{
				return;
			}

			if (data.valid() == false)
			{
				return;
			}

			if (bPreviousSentFailed)
			{
				return;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data.get()), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}

				if (res < 0)
				{
					break;
				}

				size -= res;
			} while (size > 0);
		}

		void Client::SingleSend(Net::RawData_t& data, bool& bPreviousSentFailed)
		{
			if (data.value() == nullptr)
			{
				return;
			}

			if (GetSocket() == false)
			{
				return;
			}

			if (bPreviousSentFailed)
			{
				return;
			}

			size_t size = data.size();
			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data.value()), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}

				if (res < 0)
				{
					break;
				}

				size -= res;
			} while (size > 0);
		}

		/*
		*							Visualisation of packet structure in NET
		*	---------------------------------------------------------------------------------------------------------------------------------
		*				CRYPTED VERSION					|		NON-CRYPTED VERSION
		*	---------------------------------------------------------------------------------------------------------------------------------
		*	{KEYWORD}{SIZE}DATA
		*	---------------------------------------------------------------------------------------------------------------------------------
		*	{BP}								*		{BP}
		*		{PS}{...}						*			{PS}{...}
		*	---------------------------------------------------------------------------------------------------------------------------------
		*									COMPRESSION
		*		{POS}{...}						*			{POS}{...}
		*	---------------------------------------------------------------------------------------------------------------------------------
		*			{AK}{...}...				*						-
		*			{AV}{...}...				*						-
		*			{RDK}{...}...				*				{RDK}{...}...
		*			{RD}{...}...				*				{RD}{...}...
		*			{D}{...}...					*				{D}{...}...
		*	{EP}								*		{EP}
		*
		*/
		void Client::DoSend(const int id, NET_PACKET& pkg)
		{
			if (getConnectionStatus() != ECONNECTED)
			{
				return;
			}

			std::lock_guard<std::mutex> guard(network._mutex_send);

			Net::Json::Document doc;
			doc[CSTRING("ID")] = id;
			doc[CSTRING("CONTENT")] = pkg.Data();

			auto buffer = doc.Serialize(Net::Json::SerializeType::UNFORMATTED);

			auto dataBufferSize = buffer.size();
			NET_CPOINTER<BYTE> dataBuffer(ALLOC<BYTE>(dataBufferSize + 1));
			memcpy(dataBuffer.get(), buffer.get().get(), dataBufferSize);
			dataBuffer.get()[dataBufferSize] = '\0';
			buffer.clear();

			size_t combinedSize = 0;

			// copy rawData
			std::vector<Net::RawData_t> rawData = PKG.GetRawData();
			BYTE bRawDataModified = 0;

			/* Crypt */
			if (
				(Isset(NET_OPT_USE_CIPHER) 
					? GetOption<bool>(NET_OPT_USE_CIPHER) 
					: NET_OPT_DEFAULT_USE_CIPHER) 
				&& network.RSAHandshake
				)
			{
				NET_AES aes;

				/* Generate new AES Keypair */
				size_t aesKeySize = Isset(NET_OPT_CIPHER_AES_SIZE) ? GetOption<size_t>(NET_OPT_CIPHER_AES_SIZE) : NET_OPT_DEFAULT_AES_SIZE;
				NET_CPOINTER<BYTE> Key(ALLOC<BYTE>(aesKeySize + 1));
				Random::GetRandStringNew(Key.reference().get(), aesKeySize);
				Key.get()[aesKeySize] = '\0';

				NET_CPOINTER<BYTE> IV(ALLOC<BYTE>(CryptoPP::AES::BLOCKSIZE + 1));
				Random::GetRandStringNew(IV.reference().get(), CryptoPP::AES::BLOCKSIZE);
				IV.get()[CryptoPP::AES::BLOCKSIZE] = '\0';

				if (!aes.init(reinterpret_cast<const char*>(Key.get()), reinterpret_cast<const char*>(IV.get())))
				{
					Key.free();
					IV.free();

					Disconnect();
					return;
				}

				/* Encrypt AES Keypair using RSA */
				if (network.RSA.encryptBase64(Key.reference().get(), aesKeySize) == false)
				{
					Key.free();
					IV.free();

					Disconnect();
					return;
				}

				size_t IVSize = CryptoPP::AES::BLOCKSIZE;
				if (network.RSA.encryptBase64(IV.reference().get(), IVSize) == false)
				{
					Key.free();
					IV.free();

					Disconnect();
					return;
				}

				/* Compression */
				size_t original_dataBufferSize = dataBufferSize;
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					/* Compress Data */
					CompressData(dataBuffer.reference().get(), dataBufferSize);

					/* Compress Raw Data */
					if (rawData.empty() == false)
					{
						for (auto& data : rawData)
						{
							auto pCopy = ALLOC<BYTE>(data.size());
							memcpy(pCopy, data.value(), data.size());
							data.set(pCopy);

							data.set_original_size(data.size());
							CompressData(data.value(), data.size());
						}

						bRawDataModified = 1;
					}
				}

				/* Crypt Buffer using AES and Encode to Base64 */
				aes.encrypt(dataBuffer.get(), dataBufferSize);

				if (rawData.empty() == false)
				{
					for (auto& data : rawData)
					{
						auto pCopy = ALLOC<BYTE>(data.size());
						memcpy(pCopy, data.value(), data.size());
						data.set(pCopy);

						aes.encrypt(data.value(), data.size());
					}

					bRawDataModified = 1;
				}

				combinedSize = dataBufferSize + NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + NET_DATA_LEN + NET_PACKET_FOOTER_LEN + NET_AES_KEY_LEN + NET_AES_IV_LEN + aesKeySize + IVSize + 8;

				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					combinedSize += NET_PACKET_ORIGINAL_SIZE_LEN;
					combinedSize += 2; // begin & end tag

					std::string original_dataBufferSize_str = std::to_string(original_dataBufferSize);
					combinedSize += original_dataBufferSize_str.length();
				}

				// Append Raw data packet size
				if (rawData.empty() == false)
				{
					combinedSize += PKG.CalcRawDataFulLSize(Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION, rawData);
				}

				std::string dataSizeStr = std::to_string(dataBufferSize);
				combinedSize += dataSizeStr.length();

				const auto KeySizeStr = std::to_string(aesKeySize);
				combinedSize += KeySizeStr.length();

				const auto IVSizeStr = std::to_string(IVSize);
				combinedSize += IVSizeStr.length();

				const auto EntirePacketSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

				auto bPreviousSentFailed = false;

				/* Append Packet Header */
				SingleSend(NET_PACKET_HEADER, NET_PACKET_HEADER_LEN, bPreviousSentFailed);

				// Append Packet Size Syntax
				SingleSend(NET_PACKET_SIZE, NET_PACKET_SIZE_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
				SingleSend(EntirePacketSizeStr.data(), EntirePacketSizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);

				/* Append Original Uncompressed Packet Size */
				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					const auto UnCompressedPacketSizeStr = std::to_string(original_dataBufferSize + std::to_string(original_dataBufferSize).length());

					SingleSend(NET_PACKET_ORIGINAL_SIZE, NET_PACKET_ORIGINAL_SIZE_LEN, bPreviousSentFailed);
					SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
					SingleSend(UnCompressedPacketSizeStr.data(), UnCompressedPacketSizeStr.length(), bPreviousSentFailed);
					SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				}

				/* Append Packet Key */
				SingleSend(NET_AES_KEY, NET_AES_KEY_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
				SingleSend(KeySizeStr.data(), KeySizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(Key, aesKeySize, bPreviousSentFailed);
				Key.free();

				/* Append Packet IV */
				SingleSend(NET_AES_IV, NET_AES_IV_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
				SingleSend(IVSizeStr.data(), IVSizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(IV, IVSize, bPreviousSentFailed);
				IV.free();

				/* Append Packet Data */
				if (rawData.empty() == false)
				{
					for (auto& data : rawData)
					{
						// Append Key
						SingleSend(NET_RAW_DATA_KEY, NET_RAW_DATA_KEY_LEN, bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);

						const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

						SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						SingleSend(data.key(), strlen(data.key()) + 1, bPreviousSentFailed);

						// Append Original Size
						/* Compression */
						if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						{
							const auto OriginalSizeStr = std::to_string(data.original_size() + std::to_string(data.original_size()).length());

							SingleSend(NET_RAW_DATA_ORIGINAL_SIZE, NET_RAW_DATA_ORIGINAL_SIZE_LEN, bPreviousSentFailed);
							SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
							SingleSend(OriginalSizeStr.data(), OriginalSizeStr.length(), bPreviousSentFailed);
							SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						}

						// Append Raw Data
						SingleSend(NET_RAW_DATA, NET_RAW_DATA_LEN, bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);

						const auto rawDataLengthStr = std::to_string(data.size());

						SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						SingleSend(data, bPreviousSentFailed);

						if (bRawDataModified == 1)
						{
							// original pointer still exist and was never free'd
							data.free();
						}
					}
				}

				SingleSend(NET_DATA, NET_DATA_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
				SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed);
				dataBuffer.free();

				/* Append Packet Footer */
				SingleSend(NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN, bPreviousSentFailed);
			}
			else
			{
				/* Compression */
				size_t original_dataBufferSize = dataBufferSize;
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					/* Compress Data */
					CompressData(dataBuffer.reference().get(), dataBufferSize);

					/* Compress Raw Data */
					if (PKG.HasRawData())
					{
						for (auto& data : PKG.GetRawData())
						{
							auto pCopy = ALLOC<BYTE>(data.size());
							memcpy(pCopy, data.value(), data.size());
							data.set(pCopy);

							data.set_original_size(data.size());
							CompressData(data.value(), data.size());
						}

						bRawDataModified = 1;
					}
				}

				combinedSize = dataBufferSize + NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + NET_DATA_LEN + NET_PACKET_FOOTER_LEN + 4;

				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					combinedSize += NET_PACKET_ORIGINAL_SIZE_LEN;
					combinedSize += 2; // begin & end tag

					std::string original_dataBufferSize_str = std::to_string(original_dataBufferSize);
					combinedSize += original_dataBufferSize_str.length();
				}

				// Append Raw data packet size
				if (rawData.empty() == false)
				{
					combinedSize += PKG.CalcRawDataFulLSize(Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION, rawData);
				}

				std::string dataSizeStr = std::to_string(dataBufferSize);
				combinedSize += dataSizeStr.length();

				const auto EntirePacketSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

				auto bPreviousSentFailed = false;

				/* Append Packet Header */
				SingleSend(NET_PACKET_HEADER, NET_PACKET_HEADER_LEN, bPreviousSentFailed);

				// Append Packet Size Syntax
				SingleSend(NET_PACKET_SIZE, NET_PACKET_SIZE_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
				SingleSend(EntirePacketSizeStr.data(), EntirePacketSizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);

				/* Append Original Uncompressed Packet Size */
				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					const auto UnCompressedPacketSizeStr = std::to_string(original_dataBufferSize + std::to_string(original_dataBufferSize).length());

					SingleSend(NET_PACKET_ORIGINAL_SIZE, NET_PACKET_ORIGINAL_SIZE_LEN, bPreviousSentFailed);
					SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
					SingleSend(UnCompressedPacketSizeStr.data(), UnCompressedPacketSizeStr.length(), bPreviousSentFailed);
					SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				}

				/* Append Packet Data */
				if (rawData.empty() == false)
				{
					for (auto& data : rawData)
					{
						// Append Key
						SingleSend(NET_RAW_DATA_KEY, NET_RAW_DATA_KEY_LEN, bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);

						const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

						SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						SingleSend(data.key(), strlen(data.key()) + 1, bPreviousSentFailed);

						// Append Original Size
						/* Compression */
						if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						{
							const auto OriginalSizeStr = std::to_string(data.original_size() + std::to_string(data.original_size()).length());

							SingleSend(NET_RAW_DATA_ORIGINAL_SIZE, NET_RAW_DATA_ORIGINAL_SIZE_LEN, bPreviousSentFailed);
							SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);
							SingleSend(OriginalSizeStr.data(), OriginalSizeStr.length(), bPreviousSentFailed);
							SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						}

						// Append Raw Data
						SingleSend(NET_RAW_DATA, NET_RAW_DATA_LEN, bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed);

						const auto rawDataLengthStr = std::to_string(data.size());

						SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
						SingleSend(data, bPreviousSentFailed);

						if (bRawDataModified == 1)
						{
							// original pointer still exist and was never free'd
							data.free();
						}
					}
				}

				SingleSend(NET_DATA, NET_DATA_LEN, bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_OPEN, strlen(NET_PACKET_BRACKET_OPEN), bPreviousSentFailed);
				SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed);
				SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed);
				dataBuffer.free();

				/* Append Packet Footer */
				SingleSend(NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN, bPreviousSentFailed);
			}
		}

		/*
		*							Visualisation of packet structure in NET
		*	---------------------------------------------------------------------------------------------------------------------------------
		*				CRYPTED VERSION					|		NON-CRYPTED VERSION
		*	---------------------------------------------------------------------------------------------------------------------------------
		*	{KEYWORD}{SIZE}DATA
		*	---------------------------------------------------------------------------------------------------------------------------------
		*	{BP}								*		{BP}
		*		{PS}{...}						*			{PS}{...}
		*	---------------------------------------------------------------------------------------------------------------------------------
		*									COMPRESSION
		*		{POS}{...}						*			{POS}{...}
		*	---------------------------------------------------------------------------------------------------------------------------------
		*			{AK}{...}...				*						-
		*			{AV}{...}...				*						-
		*			{RDK}{...}...				*				{RDK}{...}...
		*			{RD}{...}...				*				{RD}{...}...
		*			{D}{...}...					*				{D}{...}...
		*	{EP}								*		{EP}
		*
		*/
		DWORD Client::DoReceive()
		{
			if (getConnectionStatus() != ECONNECTED)
			{
				return -1;
			}

			auto data_size = Ws2_32::recv(GetSocket(), reinterpret_cast<char*>(network.dataReceive.get()), network.data_receive_size, 0);
			if (data_size == SOCKET_ERROR)
			{
#ifdef BUILD_LINUX
				if (errno != EWOULDBLOCK)
#else
				if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
				{
					network.ResetReceiveBuffer();

#ifdef BUILD_LINUX
					if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
#else
					if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

					return -1;
				}

				network.ResetReceiveBuffer();
				return ProcessPackets();
			}

			// graceful disconnect
			if (data_size == 0)
			{
				network.ResetReceiveBuffer();
				NET_LOG_PEER(CSTRING("Connection has been gracefully closed"));
				return -1;
			}

			if (!network.data.valid())
			{
				network.AllocData(data_size);
				memcpy(network.data.get(), network.dataReceive.get(), data_size);
				network.data.get()[data_size] = '\0';
				network.data_size = data_size;
			}
			else
			{
				if (network.data_full_size > 0
					&& network.data_size + data_size < network.data_full_size)
				{
					memcpy(&network.data.get()[network.data_size], network.dataReceive.get(), data_size);
					network.data_size += data_size;
				}
				else
				{
					/* store incomming */
					const auto newBuffer = ALLOC<BYTE>(network.data_size + data_size + 1);
					memcpy(newBuffer, network.data.get(), network.data_size);
					memcpy(&newBuffer[network.data_size], network.dataReceive.get(), data_size);
					newBuffer[network.data_size + data_size] = '\0';
					network.data.free();
					network.data = newBuffer; // pointer swap
					network.data_size += data_size;
				}
			}

			network.ResetReceiveBuffer();
			return ProcessPackets();
		}

		DWORD Client::ProcessPackets()
		{
			// check valid data size
			if (network.data_size == 0)
			{
				return 0;
			}

			if (network.data_size == INVALID_SIZE)
			{
				return 0;
			}

			if (network.data_size < NET_PACKET_HEADER_LEN)
			{
				return 0;
			}

			// [PROTOCOL] - read data full size from header
			if (
				network.data_full_size == 0 ||
				network.data_full_size == INVALID_SIZE
				)
			{
				// read entire packet size
				const size_t start = NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + 1;
				for (size_t i = start; i < network.data_size; ++i)
				{
					// iterate until we have found the end tag
					if (!memcmp(&network.data.get()[i], NET_PACKET_BRACKET_CLOSE, 1))
					{
						network.data_offset = i;
						const auto size = i - start;
						char* end = (char*)network.data.get()[start] + size;
						network.data_full_size = strtoull((const char*)&network.data.get()[start], &end, 10);

						// awaiting more bytes
						if (network.data_full_size > network.data_size)
						{
							// pre-allocate enough space
							const auto newBuffer = ALLOC<BYTE>(network.data_full_size + 1);
							memcpy(newBuffer, network.data.get(), network.data_size);
							newBuffer[network.data_full_size] = '\0';
							network.data = newBuffer; // pointer swap
							return 0;
						}

						break;
					}
				}
			}

			// keep going until we have received the entire packet
			if (
				network.data_full_size == 0 ||
				network.data_full_size == INVALID_SIZE ||
				network.data_size < network.data_full_size
				)
			{
				return 0;
			}

			/* Decompression */
			{
				/* iterate to find out if there is even a compression tag */
				size_t startPos = 0;
				for (size_t i = 0; i < network.data_size; ++i)
				{
					if (!memcmp(&network.data.get()[i], NET_PACKET_ORIGINAL_SIZE, NET_PACKET_ORIGINAL_SIZE_LEN))
					{
						startPos = i + NET_PACKET_ORIGINAL_SIZE_LEN + 1;
						break;
					}
				}

				if (startPos != 0)
				{
					for (size_t i = startPos; i < network.data_size; ++i)
					{
						// iterate until we have found the end tag
						if (!memcmp(&network.data.get()[i], NET_PACKET_BRACKET_CLOSE, 1))
						{
							network.data_offset = i;
							const auto size = i - startPos - 1;
							char* end = (char*)network.data.get()[startPos] + size;
							network.data_original_uncompressed_size = strtoull((const char*)&network.data.get()[startPos], &end, 10);

							break;
						}
					}
				}
			}

			// [PROTOCOL] - check footer is actually valid
			if (memcmp(&network.data.get()[network.data_full_size - NET_PACKET_FOOTER_LEN], NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN) != 0)
			{
				network.clear();
				NET_LOG_ERROR(CSTRING("[NET] - Received a frame with an invalid footer"));
				return -1;
			}

			// Execute the packet
			const auto ret = ExecutePacket();
			if (ret == -1)
			{
				return -1;
			}

			// re-alloc buffer
			const auto leftSize = static_cast<int>(network.data_size - network.data_full_size) > 0 ? network.data_size - network.data_full_size : INVALID_SIZE;
			if (
				leftSize != INVALID_SIZE &&
				leftSize > 0
				)
			{
				const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
				memcpy(leftBuffer, &network.data.get()[network.data_full_size], leftSize);
				leftBuffer[leftSize] = '\0';
				network.clearData();
				network.data = leftBuffer; // swap pointer
				network.data_size = leftSize;
				return 0;
			}

			network.clearData();
			return 0;
		}


		struct TPacketExcecute
		{
			Net::Packet* m_packet;
			Net::Client::Client* m_client;
			int m_packetId;
		};

		NET_THREAD(ThreadPacketExecute)
		{
			auto tpe = (TPacketExcecute*)parameter;
			if (tpe == nullptr)
			{
				return 0;
			}

			if (tpe->m_client->CheckDataN(tpe->m_packetId, *tpe->m_packet) == false)
			{
				if (tpe->m_client->CheckData(tpe->m_packetId, *tpe->m_packet) == false)
				{
					NET_LOG_PEER(CSTRING("[NET] - Frame is not defined"));
					tpe->m_client->Disconnect();
				}
			}

			if (tpe->m_packet->HasRawData())
			{
				auto& rawData = tpe->m_packet->GetRawData();
				for (auto& data : rawData)
				{
					data.free();
				}
			}

			FREE<Net::Packet>(tpe->m_packet);
			FREE<TPacketExcecute>(tpe);
			return 0;
		}

		DWORD Client::ExecutePacket()
		{
			int packetId = -1;

			NET_CPOINTER<BYTE> data(nullptr);

			Net::Packet packet;
			Net::Json::Document doc;
			DWORD ret = 0;

			/* Crypt */
			if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && network.RSAHandshake)
			{
				auto offset = network.data_offset + 1;

				NET_CPOINTER<BYTE> AESKey(nullptr);
				size_t AESKeySize;

				// look for key tag
				if (!memcmp(&network.data.get()[offset], NET_AES_KEY, NET_AES_KEY_LEN))
				{
					offset += NET_AES_KEY_LEN;

					// read size
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							AESKeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					AESKey = ALLOC<BYTE>(AESKeySize + 1);
					memcpy(AESKey.get(), &network.data.get()[offset], AESKeySize);
					AESKey.get()[AESKeySize] = '\0';

					offset += AESKeySize;
				}

				NET_CPOINTER<BYTE> AESIV(nullptr);
				size_t AESIVSize;

				// look for iv tag
				if (!memcmp(&network.data.get()[offset], NET_AES_IV, strlen(NET_AES_IV)))
				{
					offset += strlen(NET_AES_IV);

					// read size
					for (auto y = offset; y < network.data_size; ++y)
					{
						if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
						{
							const auto psize = y - offset - 1;
							NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
							memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
							dataSizeStr.get()[psize] = '\0';
							AESIVSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
							dataSizeStr.free();

							offset += psize + 2;
							break;
						}
					}

					// read the data
					AESIV = ALLOC<BYTE>(AESIVSize + 1);
					memcpy(AESIV.get(), &network.data.get()[offset], AESIVSize);
					AESIV.get()[AESIVSize] = '\0';

					offset += AESIVSize;
				}

				if (!network.RSA.decryptBase64(AESKey.reference().get(), AESKeySize))
				{
					AESKey.free();
					AESIV.free();
					NET_LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-Key & RSA and Base64"));

					ret = -1;
					goto loc_packet_free;
				}

				if (!network.RSA.decryptBase64(AESIV.reference().get(), AESIVSize))
				{
					AESKey.free();
					AESIV.free();
					NET_LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-IV & RSA and Base64"));

					ret = -1;
					goto loc_packet_free;
				}

				NET_AES aes;
				if (!aes.init(reinterpret_cast<const char*>(AESKey.get()), reinterpret_cast<const char*>(AESIV.get())))
				{
					AESKey.free();
					AESIV.free();
					NET_LOG_ERROR(CSTRING("[NET] - Initializing AES failure"));

					ret = -1;
					goto loc_packet_free;
				}

				AESKey.free();
				AESIV.free();

				do
				{
					// look for raw data tag
					if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, NET_RAW_DATA_KEY_LEN))
					{
						offset += NET_RAW_DATA_KEY_LEN;

						// read size
						NET_CPOINTER<BYTE> key(nullptr);
						size_t KeySize = 0;
						{
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}

							// read the data
							key = ALLOC<BYTE>(KeySize + 1);
							memcpy(key.get(), &network.data.get()[offset], KeySize);
							key.get()[KeySize] = '\0';

							offset += KeySize;
						}

						// looking for raw data original size tag
						/* Decompression */
						size_t originalSize = 0;
						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_ORIGINAL_SIZE, NET_RAW_DATA_ORIGINAL_SIZE_LEN))
						{
							offset += NET_RAW_DATA_ORIGINAL_SIZE_LEN;

							// read original size
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									originalSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}
						}

						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, NET_RAW_DATA_LEN))
						{
							offset += NET_RAW_DATA_LEN;

							// read size
							size_t packetSize = 0;
							{
								for (auto y = offset; y < network.data_size; ++y)
								{
									if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
									{
										const auto psize = y - offset - 1;
										NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
										memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
										dataSizeStr.get()[psize] = '\0';
										packetSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
										dataSizeStr.free();

										offset += psize + 2;
										break;
									}
								}
							}

							Net::RawData_t entry = { (char*)key.get(), &network.data.get()[offset], packetSize, false };

							// create copy
							BYTE* copy = ALLOC<BYTE>(entry.size());
							memcpy(copy, entry.value(), entry.size());
							entry.set(copy);

							/* decrypt aes */
							if (aes.decrypt(entry.value(), entry.size()) == false)
							{
								entry.free();

								NET_LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));

								ret = -1;
								goto loc_packet_free;
							}

							/* Decompression */
							if (originalSize != 0)
							{
								entry.set_original_size(originalSize);
								DecompressData(entry.value(), entry.size(), entry.original_size());
								entry.set_original_size(0);
							}

							packet.AddRaw(entry);
							key.free();

							offset += packetSize;
						}
					}

					// look for data tag
					if (!memcmp(&network.data.get()[offset], NET_DATA, NET_DATA_LEN))
					{
						offset += NET_DATA_LEN;

						// read size
						size_t packetSize = 0;
						{
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									packetSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}
						}

						// read the data
						data = ALLOC<BYTE>(packetSize + 1);
						memcpy(data.get(), &network.data.get()[offset], packetSize);
						data.get()[packetSize] = '\0';

						offset += packetSize;

						/* decrypt aes */
						if (aes.decrypt(data.get(), packetSize) == false)
						{
							NET_LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));

							ret = -1;
							goto loc_packet_free;
						}

						/* Decompression */
						if (network.data_original_uncompressed_size != 0)
						{
							DecompressData(data.reference().get(), packetSize, network.data_original_uncompressed_size);
							network.data_original_uncompressed_size = 0;
						}
					}

					// we have reached the end of reading
					if (offset + NET_PACKET_FOOTER_LEN >= network.data_full_size)
					{
						break;
					}
				} while (1);
			}
			else
			{
				auto offset = network.data_offset + 1;

				do
				{
					// look for raw data tag
					if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, NET_RAW_DATA_KEY_LEN))
					{
						offset += NET_RAW_DATA_KEY_LEN;

						// read size
						NET_CPOINTER<BYTE> key(nullptr);
						size_t KeySize = 0;
						{
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									KeySize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}

							// read the data
							key = ALLOC<BYTE>(KeySize + 1);
							memcpy(key.get(), &network.data.get()[offset], KeySize);
							key.get()[KeySize] = '\0';

							offset += KeySize;
						}

						// looking for raw data original size tag
						/* Decompression */
						size_t originalSize = 0;
						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_ORIGINAL_SIZE, NET_RAW_DATA_ORIGINAL_SIZE_LEN))
						{
							offset += NET_RAW_DATA_ORIGINAL_SIZE_LEN;

							// read original size
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									originalSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}
						}

						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, NET_RAW_DATA_LEN))
						{
							offset += NET_RAW_DATA_LEN;

							// read size
							size_t packetSize = 0;
							{
								for (auto y = offset; y < network.data_size; ++y)
								{
									if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
									{
										const auto psize = y - offset - 1;
										NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
										memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
										dataSizeStr.get()[psize] = '\0';
										packetSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
										dataSizeStr.free();

										offset += psize + 2;
										break;
									}
								}
							}

							Net::RawData_t entry = { (char*)key.get(), &network.data.get()[offset], packetSize, false };

							// create copy
							BYTE* copy = ALLOC<BYTE>(entry.size());
							memcpy(copy, entry.value(), entry.size());
							entry.set(copy);

							/* Decompression */
							if (originalSize != 0)
							{
								entry.set_original_size(originalSize);
								DecompressData(entry.value(), entry.size(), entry.original_size());
								entry.set_original_size(entry.size());
							}

							packet.AddRaw(entry);
							key.free();

							offset += packetSize;
						}
					}

					// look for data tag
					if (!memcmp(&network.data.get()[offset], NET_DATA, NET_DATA_LEN))
					{
						offset += NET_DATA_LEN;

						// read size
						size_t packetSize = 0;
						{
							for (auto y = offset; y < network.data_size; ++y)
							{
								if (!memcmp(&network.data.get()[y], NET_PACKET_BRACKET_CLOSE, 1))
								{
									const auto psize = y - offset - 1;
									NET_CPOINTER<BYTE> dataSizeStr(ALLOC<BYTE>(psize + 1));
									memcpy(dataSizeStr.get(), &network.data.get()[offset + 1], psize);
									dataSizeStr.get()[psize] = '\0';
									packetSize = strtoull(reinterpret_cast<const char*>(dataSizeStr.get()), nullptr, 10);
									dataSizeStr.free();

									offset += psize + 2;
									break;
								}
							}
						}

						// read the data
						data = ALLOC<BYTE>(packetSize + 1);
						memcpy(data.get(), &network.data.get()[offset], packetSize);
						data.get()[packetSize] = '\0';

						offset += packetSize;

						/* Compression */
						if (network.data_original_uncompressed_size != 0)
						{
							DecompressData(data.reference().get(), packetSize, network.data_original_uncompressed_size);
							network.data_original_uncompressed_size = 0;
						}
					}

					// we have reached the end of reading
					if (offset + NET_PACKET_FOOTER_LEN >= network.data_full_size)
					{
						break;
					}
				} while (1);
			}

			if (data.valid() == false)
			{
				NET_LOG_PEER(CSTRING("[NET] - JSON data is not valid"));

				ret = -1;
				goto loc_packet_free;
			}

			/*
			* parse json
			* get packet id from it
			* and json content
			*
			* pass the json content into packet object
			*/
			printf("%s\n\t", data.get());
			if (doc.Deserialize(reinterpret_cast<char*>(data.get())) == false)
			{
				NET_LOG_PEER(CSTRING("[NET] - Unable to deserialize json data"));

				ret = -1;
				data.Set(nullptr);
				goto loc_packet_free;
			}

			data.Set(nullptr);

			{
				auto s = doc.Serialize();
				printf("%s\n\t", s.get().get());
			}

			if (
				doc[CSTRING("ID")] == 0 ||
				doc[CSTRING("ID")]->is_int() == 0
				)
			{
				NET_LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));

				ret = -1;
				goto loc_packet_free;
			}

			packetId = doc[CSTRING("ID")]->as_int();
			if (packetId < 0)
			{
				NET_LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));

				ret = -1;
				goto loc_packet_free;
			}

			if (doc[CSTRING("CONTENT")] != 0)
			{
				if (doc[CSTRING("CONTENT")]->is_object())
				{
					packet.Data().Set(doc[CSTRING("CONTENT")]->as_object());
				}
				else if (doc[CSTRING("CONTENT")]->is_array())
				{
					packet.Data().Set(doc[CSTRING("CONTENT")]->as_array());
				}
				else
				{
					NET_LOG_PEER(CSTRING("[NET] - packet is wrong format"));

					ret = -1;
					goto loc_packet_free;
				}
			}

			// check for option async to execute the packet in a new created thread
			if (Isset(NET_OPT_EXECUTE_PACKET_ASYNC) ? GetOption<bool>(NET_OPT_EXECUTE_PACKET_ASYNC) : NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC)
			{
				auto tpe = ALLOC<TPacketExcecute>();
				tpe->m_packet = ALLOC<Net::Packet, Net::Packet>(1, packet);
				tpe->m_client = this;
				tpe->m_packetId = packetId;
				const auto hThread = Net::Thread::Create(ThreadPacketExecute, tpe);
				if (hThread)
				{
					// Close only closes handle, it does not close the thread
					Net::Thread::Close(hThread);
					return 0;
				}

				FREE<TPacketExcecute>(tpe);
			}

			// execute packet in this thread
			if (CheckDataN(packetId, packet) == false)
			{
				if (CheckData(packetId, packet) == false)
				{
					ret = -1;
					NET_LOG_PEER(CSTRING("[NET] - Frame is not defined"));
				}
			}

		loc_packet_free:
			data.free();

			if (packet.HasRawData())
			{
				auto& rawData = packet.GetRawData();
				for (auto& data : rawData)
				{
					data.free();
				}
			}

			return ret;
		}

		void Client::CompressData(BYTE*& data, size_t& size)
		{
#ifdef DEBUG
			const auto PrevSize = size;
#endif

			BYTE* m_pCompressed = 0;
			size_t m_iCompressedLen = 0;
			NET_ZLIB::Compress(data, size, m_pCompressed, m_iCompressedLen);
			FREE<BYTE>(data);
			data = m_pCompressed;
			size = m_iCompressedLen;

#ifdef DEBUG
			NET_LOG_DEBUG(CSTRING("[NET] - Compressed data from size %llu to %llu"), PrevSize, size);
#endif

			/* base64 encode it */
			NET_BASE64::encode(data, size);
		}

		void Client::CompressData(BYTE*& data, BYTE*& out, size_t& size, const bool skip_free)
		{
#ifdef DEBUG
			const auto PrevSize = size;
#endif

			BYTE* m_pCompressed = 0;
			size_t m_iCompressedLen = 0;
			NET_ZLIB::Compress(data, size, m_pCompressed, m_iCompressedLen);

			if (!skip_free)
			{
				FREE<BYTE>(data);
			}

			out = m_pCompressed;
			size = m_iCompressedLen;

#ifdef DEBUG
			NET_LOG_DEBUG(CSTRING("[NET] - Compressed data from size %llu to %llu"), PrevSize, size);
#endif

			/* base64 encode it */
			NET_BASE64::encode(out, size);
		}

		void Client::DecompressData(BYTE*& data, size_t& size, size_t original_size)
		{
			/* base64 decode it */
			NET_BASE64::decode(data, size);

#ifdef DEBUG
			const auto PrevSize = size;
#endif

			BYTE* m_pUnCompressed = 0;
			size_t m_iUncompressedLen = original_size;
			NET_ZLIB::Decompress(data, size, m_pUnCompressed, m_iUncompressedLen);
			FREE<BYTE>(data);
			data = m_pUnCompressed;
			size = m_iUncompressedLen;

#ifdef DEBUG
			NET_LOG_DEBUG(CSTRING("[NET] - Decompressed data from size %llu to %llu"), PrevSize, size);
#endif
		}

		void Client::DecompressData(BYTE*& data, BYTE*& out, size_t& size, size_t original_size, const bool skip_free)
		{
			/* base64 decode it */
			NET_BASE64::decode(data, size);

#ifdef DEBUG
			const auto PrevSize = size;
#endif

			BYTE* m_pUnCompressed = 0;
			size_t m_iUncompressedLen = original_size;
			NET_ZLIB::Decompress(data, size, m_pUnCompressed, m_iUncompressedLen);

			if (!skip_free)
			{
				FREE<BYTE>(data);
			}

			out = m_pUnCompressed;
			size = m_iUncompressedLen;

#ifdef DEBUG
			NET_LOG_DEBUG(CSTRING("[NET] - Decompressed data from size %llu to %llu"), PrevSize, size);
#endif
		}

		NET_NATIVE_PACKET_DEFINITION_BEGIN(Client);
		NET_DEFINE_PACKET(NetProtocolHandshake, NET_NATIVE_PACKET_ID::PKG_NetProtocolHandshake);
		NET_DEFINE_PACKET(RSAHandshake, NET_NATIVE_PACKET_ID::PKG_NetAsymmetricHandshake);
		NET_DEFINE_PACKET(EstabilishConnection, NET_NATIVE_PACKET_ID::PKG_NetEstabilish);
		NET_DEFINE_PACKET(Close, NET_NATIVE_PACKET_ID::PKG_NetClose);
		NET_DEFINE_PACKET(NetHeartbeat, NET_NATIVE_PACKET_ID::PKG_NetHeartbeat);
		NET_PACKET_DEFINITION_END;

		NET_BEGIN_PACKET(Client, NetProtocolHandshake);
		if (PKG[CSTRING("NET_OPT_USE_CIPHER")] && PKG[CSTRING("NET_OPT_USE_CIPHER")]->is_boolean())
		{
			this->SetOption<bool>({ NET_OPT_USE_CIPHER, PKG[CSTRING("NET_OPT_USE_CIPHER")]->as_boolean() });

			if (PKG[CSTRING("NET_OPT_CIPHER_RSA_SIZE")] && PKG[CSTRING("NET_OPT_CIPHER_RSA_SIZE")]->is_int())
			{
				this->SetOption<size_t>({ NET_OPT_CIPHER_RSA_SIZE, (size_t)PKG[CSTRING("NET_OPT_CIPHER_RSA_SIZE")]->as_int() });
			}

			if (PKG[CSTRING("NET_OPT_CIPHER_AES_SIZE")] && PKG[CSTRING("NET_OPT_CIPHER_AES_SIZE")]->is_int())
			{
				this->SetOption<size_t>({ NET_OPT_CIPHER_AES_SIZE, (size_t)PKG[CSTRING("NET_OPT_CIPHER_AES_SIZE")]->as_int() });
			}

			/* create new keys */
			network.createNewRSAKeys(Isset(NET_OPT_CIPHER_RSA_SIZE) ? GetOption<size_t>(NET_OPT_CIPHER_RSA_SIZE) : NET_OPT_DEFAULT_RSA_SIZE);
		}

		if (PKG[CSTRING("NET_OPT_USE_COMPRESSION")] && PKG[CSTRING("NET_OPT_USE_COMPRESSION")]->is_boolean())
		{
			this->SetOption<bool>({ NET_OPT_USE_COMPRESSION, PKG[CSTRING("NET_OPT_USE_COMPRESSION")]->as_boolean() });
		}

		/* communication set */
		// -> Response to Server with current Net Version
		Net::Packet resp;
		resp[CSTRING("NET_STATUS")] = 1;
		resp[CSTRING("NET_MAJOR_VERSION")] = Version::Major();
		resp[CSTRING("NET_MINOR_VERSION")] = Version::Minor();
		resp[CSTRING("NET_REVISION_VERSION")] = Version::Revision();
		const auto Key = Version::Key().get();
		resp[CSTRING("NET_KEY")] = Key.get();
		NET_SEND(NET_NATIVE_PACKET_ID::PKG_NetProtocolHandshake, resp);
		NET_END_PACKET;

		NET_BEGIN_PACKET(Client, RSAHandshake);
		if (!(Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER))
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, cipher option is been disabled, rejecting the frame"), FUNCTION_NAME);
			return;
		}
		if (network.estabilished)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
			return;
		}
		if (network.RSAHandshake)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, client has already performed a handshake, rejecting the frame"), FUNCTION_NAME);
			return;
		}

		if (!(PKG[CSTRING("PublicKey")] && PKG[CSTRING("PublicKey")]->is_string()))
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, received public key is not valid, rejecting the frame"), FUNCTION_NAME);
			return;
		}

		// send our generated Public Key to the Server
		{
			auto MyPublicKey = network.RSA.publicKey();

			size_t b64len = MyPublicKey.size();
			BYTE* b64 = ALLOC<BYTE>(b64len + 1);
			memcpy(b64, MyPublicKey.data(), b64len);
			b64[b64len] = 0;

			Net::Coding::Base64::encode(b64, b64len);

			NET_PACKET resp;
			resp[CSTRING("PublicKey")] = reinterpret_cast<char*>(b64);
			NET_SEND(NET_NATIVE_PACKET_ID::PKG_NetAsymmetricHandshake, resp);

			FREE<byte>(b64);
		}

		// from now we use the Cryption, synced with Server
		{
			auto b64len = strlen(PKG[CSTRING("PublicKey")]->as_string());
			auto b64 = ALLOC<BYTE>(b64len + 1);
			memcpy(b64, PKG[CSTRING("PublicKey")]->as_string(), b64len);
			b64[b64len] = 0;

			Net::Coding::Base64::decode(b64, b64len);

			network.RSA.setPublicKey(reinterpret_cast<char*>(b64));
			network.RSAHandshake = true;
		}
		NET_END_PACKET;

		NET_BEGIN_PACKET(Client, EstabilishConnection);
		if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && !network.RSAHandshake)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received an estabilishing frame, client has not performed a handshake yet, rejecting the frame"), FUNCTION_NAME);
			return;
		}
		if (network.estabilished)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received an estabilishing frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
			return;
		}

		network.estabilished = true;

		// Callback
		// connection has been estabilished, now call entry function
		OnConnectionEstabilished();
		NET_END_PACKET;

		NET_BEGIN_PACKET(Client, Close);
		// connection has been closed
		Disconnect();

		NET_LOG_SUCCESS(CSTRING("[NET] - Connection has been closed by the Server"));

		if (!(PKG[CSTRING("code")] && PKG[CSTRING("code")]->is_int()))
		{
			// Callback
			OnConnectionClosed(-1);
			return;
		}

		const auto code = PKG[CSTRING("code")]->as_int();

		// callback Different Version
		if (code == NET_ERROR_CODE::NET_ERR_Versionmismatch)
		{
			OnVersionMismatch();
		}

		// Callback
		OnConnectionClosed(code);
		NET_END_PACKET;

		NET_BEGIN_PACKET(Client, NetHeartbeat);
		if (network.m_heartbeat_sequence_number >= INT_MAX)
			network.m_heartbeat_sequence_number = -1;

		Net::Packet resp;
		resp[CSTRING("NET_SEQUENCE_NUMBER")] = ++network.m_heartbeat_sequence_number;
		NET_SEND(NET_NATIVE_PACKET_ID::PKG_NetHeartbeat, resp);
		NET_END_PACKET;
	}
}
