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

		NET_TIMER(CalcLatency)
		{
			const auto client = (Client*)param;
			if (!client) NET_STOP_TIMER;

			if (!client->IsConnected()) NET_CONTINUE_TIMER;

			client->network.bLatency = true;
			// tmp disabled due to missing linux support
			//client->network.latency = Net::Protocol::ICMP::Exec(client->GetServerAddress());
			client->network.bLatency = false;

			Timer::SetTime(client->network.hCalcLatency, client->Isset(NET_OPT_INTERVAL_LATENCY) ? client->GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY);
			NET_CONTINUE_TIMER;
		}

		NET_TIMER(NTPSyncClock)
		{
			const auto client = (Client*)param;
			if (!client) NET_STOP_TIMER;
			if (!client->IsConnected()) NET_CONTINUE_TIMER;

			tm* tm = localtime(&client->network.curTime);
			tm->tm_sec += 1;
			client->network.curTime = mktime(tm);
			NET_CONTINUE_TIMER;
		}

		NET_TIMER(NTPReSyncClock)
		{
			const auto client = (Client*)param;
			if (!client) NET_STOP_TIMER;

			if (!client->IsConnected()) NET_CONTINUE_TIMER;

			auto time = Net::Protocol::NTP::Exec(client->Isset(NET_OPT_NTP_HOST) ? client->GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
				client->Isset(NET_OPT_NTP_PORT) ? client->GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

			if (!time.valid())
			{
				NET_LOG_ERROR(CSTRING("[NET] - critical failure on calling NTP host"));
				NET_CONTINUE_TIMER;
			}

			client->network.curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
			Timer::SetTime(client->network.hReSyncClockNTP, client->Isset(NET_OPT_NTP_SYNC_INTERVAL) ? client->GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL);
			NET_CONTINUE_TIMER;
		}

		Client::Client()
		{
			SetSocket(INVALID_SOCKET);
			SetServerAddress(CSTRING(""));
			SetServerPort(0);
			SetConnected(false);
			optionBitFlag = 0;
			socketOptionBitFlag = 0;
			bReceiveThread = false;
		}

		Client::~Client()
		{
			while (bReceiveThread)
			{
#ifdef BUILD_LINUX
				usleep(FREQUENZ * 1000);
#else
				Kernel32::Sleep(FREQUENZ);
#endif
			}

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
			if (!client) return 0;

			client->bReceiveThread = true;

			NET_LOG_DEBUG(CSTRING("[NET] - Receive thread has been started"));
			while (client->IsConnected())
			{
#ifdef BUILD_LINUX
				usleep(client->DoReceive() * 1000);
#else
				Kernel32::Sleep(client->DoReceive());
#endif
			}

			// wait until thread has finished
			while (client && client->network.bLatency)
			{
#ifdef BUILD_LINUX
				usleep(client->Isset(NET_OPT_FREQUENZ) ? client->GetOption<DWORD>(NET_OPT_FREQUENZ) * 1000 : NET_OPT_DEFAULT_FREQUENZ * 1000);
#else
				Kernel32::Sleep(client->Isset(NET_OPT_FREQUENZ) ? client->GetOption<DWORD>(NET_OPT_FREQUENZ) : NET_OPT_DEFAULT_FREQUENZ);
#endif
			}

			client->bReceiveThread = false;

			NET_LOG_DEBUG(CSTRING("[NET] - Receive thread has been end"));
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

		bool Client::ChangeMode(const bool blocking)
		{
			auto ret = true;
			unsigned long non_blocking = (blocking ? 0 : 1);
			ret = (NET_NO_ERROR == Ws2_32::ioctlsocket(GetSocket(), FIONBIO, &non_blocking)) ? true : false;
			return ret;
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
			if (IsConnected())
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
#ifdef BUILD_LINUX
									usleep(FREQUENZ * 1000);
#else
									Kernel32::Sleep(FREQUENZ);
#endif
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
#ifdef BUILD_LINUX
								usleep(FREQUENZ * 1000);
#else
								Kernel32::Sleep(FREQUENZ);
#endif
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
				NET_LOG_ERROR(CSTRING("[Client] - socket failed with error: %ld"), LAST_ERROR);

#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			/* Connect to the server */
			if (Ws2_32::connect(GetSocket(), sockaddr, slen) == SOCKET_ERROR)
			{
				SetSocket(INVALID_SOCKET);

				NET_LOG_ERROR(CSTRING("[Client] - failure on connecting to host: %s:%hu"), GetServerAddress(), GetServerPort());
#ifndef BUILD_LINUX
				Ws2_32::WSACleanup();
#endif
				return false;
			}

			// clear the unused vector
			socketoption.clear();

			// successfully connected
			SetConnected(true);

			// Set Mode
			ChangeMode(Isset(NET_OPT_MODE_BLOCKING) ? GetOption<bool>(NET_OPT_MODE_BLOCKING) : NET_OPT_DEFAULT_MODE_BLOCKING);

			/* Set Read Timeout */
			timeval tv = {};
			tv.tv_sec = Isset(NET_OPT_TIMEOUT_TCP_READ) ? GetOption<long>(NET_OPT_TIMEOUT_TCP_READ) : NET_OPT_DEFAULT_TIMEOUT_TCP_READ;
			tv.tv_usec = 0;
			Ws2_32::setsockopt(GetSocket(), SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof tv);

			// Set socket options
			for (const auto& entry : socketoption)
			{
				const auto res = Ws2_32::setsockopt(GetSocket(), entry->level, entry->opt, entry->value(), entry->optlen());
				if (res == SOCKET_ERROR) NET_LOG_ERROR(CSTRING("Following socket option could not been applied { %i : %i }"), entry->opt, LAST_ERROR);
			}

			if (Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER)
				/* create RSA Key Pair */
				network.createNewRSAKeys(Isset(NET_OPT_CIPHER_RSA_SIZE) ? GetOption<size_t>(NET_OPT_CIPHER_RSA_SIZE) : NET_OPT_DEFAULT_RSA_SIZE);

			network.hCalcLatency = Timer::Create(CalcLatency, Isset(NET_OPT_INTERVAL_LATENCY) ? GetOption<int>(NET_OPT_INTERVAL_LATENCY) : NET_OPT_DEFAULT_INTERVAL_LATENCY, this);

			// if we use NTP execute the needed code
			if (CreateTOTPSecret())
			{
				NET_LOG_DEBUG(CSTRING("[NET] - Successfully created TOTP-Hash"));
			}

			// spawn timer thread to sync clock with ntp - only effects having 2-step enabled
			if ((Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
				&& (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP))
			{
				network.hSyncClockNTP = Timer::Create(NTPSyncClock, 1000, this);
				network.hReSyncClockNTP = Timer::Create(NTPReSyncClock, Isset(NET_OPT_NTP_SYNC_INTERVAL) ? GetOption<int>(NET_OPT_NTP_SYNC_INTERVAL) : NET_OPT_DEFAULT_NTP_SYNC_INTERVAL, this);
			}

			// Create Loop-Receive Thread
			Thread::Create(Receive, this);

			// callback
			OnConnected();

			return true;
		}

		bool Client::Disconnect()
		{
			/*
			* NET_OPT_EXECUTE_PACKET_ASYNC allow packet execution in different threads
			* that threads might call Disconnect
			* soo require a mutex to block it
			*/
			std::lock_guard<std::mutex> guard(this->_mutex_disconnect);

			if (!IsConnected())
			{
				return false;
			}

			// connection has been closed
			ConnectionClosed();

			// callback
			OnDisconnected();

			NET_LOG_SUCCESS(CSTRING("[NET] - Disconnected from server"));
			return true;
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
#ifdef BUILD_LINUX
							usleep(FREQUENZ * 1000);
#else
							Kernel32::Sleep(FREQUENZ);
#endif
						}
					}

				} while (bBlocked);

				SetSocket(INVALID_SOCKET);
			}

			network.latency = -1;
			network.bLatency = false;

			if (network.hCalcLatency)
			{
				Timer::WaitSingleObjectStopped(network.hCalcLatency);
				network.hCalcLatency = nullptr;
			}

			if (network.hSyncClockNTP)
			{
				Timer::WaitSingleObjectStopped(network.hSyncClockNTP);
				network.hSyncClockNTP = nullptr;
			}

			if (network.hReSyncClockNTP)
			{
				Timer::WaitSingleObjectStopped(network.hReSyncClockNTP);
				network.hReSyncClockNTP = nullptr;
			}

			SetConnected(false);
		}

		void Client::Clear()
		{
			if (IsConnected())
			{
				NET_LOG_ERROR(CSTRING("[NET] - Can not clear Client while being connected!"));
				return;
			}

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

		void Client::SetConnected(const bool connected)
		{
			this->connected = connected;
		}

		bool Client::IsConnected() const
		{
			return connected;
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

			FREE<byte>(totp_secret);
			totp_secret_len = 0;
			curToken = 0;
			lastToken = 0;
			curTime = 0;
			hSyncClockNTP = nullptr;
			hReSyncClockNTP = nullptr;
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

		typeLatency Client::Network::getLatency() const
		{
			return latency;
		}

		void Client::SingleSend(const char* data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
		{
			if (!GetSocket())
				return;

			if (bPreviousSentFailed)
				return;

			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				char* ptr = (char*)data;
				for (size_t it = 0; it < size; ++it)
					ptr[it] = ptr[it] ^ sendToken;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						usleep(FREQUENZ * 1000);
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
						Kernel32::Sleep(FREQUENZ);
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
					break;

				size -= res;
			} while (size > 0);
		}

		void Client::SingleSend(BYTE*& data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
		{
			if (!GetSocket())
			{
				FREE<byte>(data);
				return;
			}

			if (bPreviousSentFailed)
			{
				FREE<byte>(data);
				return;
			}

			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				for (size_t it = 0; it < size; ++it)
					data[it] = data[it] ^ sendToken;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						usleep(FREQUENZ * 1000);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						FREE(data);
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						Kernel32::Sleep(FREQUENZ);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						FREE<byte>(data);
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}
				if (res < 0)
					break;

				size -= res;
			} while (size > 0);

			FREE<byte>(data);
		}

		void Client::SingleSend(NET_CPOINTER<BYTE>& data, size_t size, bool& bPreviousSentFailed, const uint32_t sendToken)
		{
			if (!GetSocket())
			{
				data.free();
				return;
			}

			if (bPreviousSentFailed)
			{
				data.free();
				return;
			}

			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				for (size_t it = 0; it < size; ++it)
					data.get()[it] = data.get()[it] ^ sendToken;
			}

			do
			{
				const auto res = Ws2_32::send(GetSocket(), reinterpret_cast<const char*>(data.get()), size, MSG_NOSIGNAL);
				if (res == SOCKET_ERROR)
				{
#ifdef BUILD_LINUX
					if (errno == EWOULDBLOCK)
					{
						usleep(FREQUENZ * 1000);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						data.free();
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						Kernel32::Sleep(FREQUENZ);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						data.free();
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}
				if (res < 0)
					break;

				size -= res;
			} while (size > 0);

			data.free();
		}

		void Client::SingleSend(Net::RawData_t& data, bool& bPreviousSentFailed, const uint32_t sendToken)
		{
			if (!data.valid()) return;

			if (!GetSocket())
			{
				data.free();
				return;
			}

			if (bPreviousSentFailed)
			{
				data.free();
				return;
			}

			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				for (size_t it = 0; it < data.size(); ++it)
					data.value()[it] = data.value()[it] ^ sendToken;
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
						usleep(FREQUENZ * 1000);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						data.free();
						Disconnect();
						if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
						return;
					}
#else
					if (Ws2_32::WSAGetLastError() == WSAEWOULDBLOCK)
					{
						Kernel32::Sleep(FREQUENZ);
						continue;
					}
					else
					{
						bPreviousSentFailed = true;
						data.free();
						Disconnect();
						if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
						return;
					}
#endif
				}
				if (res < 0)
					break;

				size -= res;
			} while (size > 0);

			data.free();
		}

		/*
		*							Visualisation of packet structure in NET
		*	------------------------------------------------------------------------------------------
		*				CRYPTED VERSION					|		NON-CRYPTED VERSION
		*	------------------------------------------------------------------------------------------
		*	{KEYWORD}{SIZE}DATA
		*	------------------------------------------------------------------------------------------
		*	{BEGIN PACKET}								*		{BEGIN PACKET}
		*		{PACKET SIZE}{...}						*			{PACKET SIZE}{...}
		*			{KEY}{...}...						*						-
		*			{IV}{...}...						*						-
		*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
		*			{RAW DATA}{...}...					*				{RAW DATA}{...}...
		*			{DATA}{...}...						*				{DATA}{...}...
		*	{END PACKET}								*		{END PACKET}
		*
		*/
		void Client::DoSend(const int id, NET_PACKET& pkg)
		{
			if (!IsConnected())
				return;

			std::lock_guard<std::mutex> guard(network._mutex_send);

			uint32_t sendToken = INVALID_UINT_SIZE;
			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
				sendToken = Net::Coding::TOTP::generateToken(network.totp_secret, network.totp_secret_len, Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP ? network.curTime : time(nullptr), Isset(NET_OPT_TOTP_INTERVAL) ? (int)(GetOption<int>(NET_OPT_TOTP_INTERVAL) / 2) : (int)(NET_OPT_DEFAULT_TOTP_INTERVAL / 2));

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

			/* Crypt */
			if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && network.RSAHandshake)
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

					NET_LOG_ERROR(CSTRING("[NET] - Failed to Init AES [0]"));
					Disconnect();
					return;
				}

				/* Encrypt AES Keypair using RSA */
				if (!network.RSA.encryptBase64(Key.reference().get(), aesKeySize))
				{
					Key.free();
					IV.free();
					NET_LOG_ERROR(CSTRING("[NET] - Failed Key to encrypt and encode to base64"));
					Disconnect();
					return;
				}

				size_t IVSize = CryptoPP::AES::BLOCKSIZE;
				if (!network.RSA.encryptBase64(IV.reference().get(), IVSize))
				{
					Key.free();
					IV.free();
					NET_LOG_ERROR(CSTRING("[NET] - Failed IV to encrypt and encode to base64"));
					Disconnect();
					return;
				}

				/* Compression */
				size_t original_dataBufferSize = dataBufferSize;
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					/* Compress Data */
					CompressData(dataBuffer.reference().get(), dataBufferSize);
				}

				/* Crypt Buffer using AES and Encode to Base64 */
				aes.encrypt(dataBuffer.get(), dataBufferSize);

				if (PKG.HasRawData())
				{
					std::vector<Net::RawData_t>& rawData = PKG.GetRawData();
					for (auto& data : rawData)
						aes.encrypt(data.value(), data.size());
				}

				combinedSize = dataBufferSize + NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + NET_DATA_LEN + NET_PACKET_FOOTER_LEN + NET_AES_KEY_LEN + strlen(NET_AES_IV) + aesKeySize + IVSize + 8;

				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					combinedSize += NET_UNCOMPRESSED_PACKET_SIZE_LEN;
					combinedSize += 2; // begin & end tag

					std::string original_dataBufferSize_str = std::to_string(original_dataBufferSize);
					combinedSize += original_dataBufferSize_str.length();
				}

				// Append Raw data packet size
				if (PKG.HasRawData()) combinedSize += PKG.GetRawDataFullSize();

				std::string dataSizeStr = std::to_string(dataBufferSize);
				combinedSize += dataSizeStr.length();

				const auto KeySizeStr = std::to_string(aesKeySize);
				combinedSize += KeySizeStr.length();

				const auto IVSizeStr = std::to_string(IVSize);
				combinedSize += IVSizeStr.length();

				const auto EntirePacketSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

				auto bPreviousSentFailed = false;

				/* Append Packet Header */
				SingleSend(NET_PACKET_HEADER, NET_PACKET_HEADER_LEN, bPreviousSentFailed, sendToken);

				// Append Packet Size Syntax
				SingleSend(NET_PACKET_SIZE, NET_PACKET_SIZE_LEN, bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
				SingleSend(EntirePacketSizeStr.data(), EntirePacketSizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

				/* Append Original Uncompressed Packet Size */
				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					const auto UnCompressedPacketSizeStr = std::to_string(original_dataBufferSize + std::to_string(original_dataBufferSize).length());

					SingleSend(NET_UNCOMPRESSED_PACKET_SIZE, NET_UNCOMPRESSED_PACKET_SIZE_LEN, bPreviousSentFailed, sendToken);
					SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
					SingleSend(UnCompressedPacketSizeStr.data(), UnCompressedPacketSizeStr.length(), bPreviousSentFailed, sendToken);
					SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				}

				/* Append Packet Key */
				SingleSend(NET_AES_KEY, NET_AES_KEY_LEN, bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
				SingleSend(KeySizeStr.data(), KeySizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(Key, aesKeySize, bPreviousSentFailed, sendToken);

				/* Append Packet IV */
				SingleSend(NET_AES_IV, strlen(NET_AES_IV), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
				SingleSend(IVSizeStr.data(), IVSizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(IV, IVSize, bPreviousSentFailed, sendToken);

				/* Append Packet Data */
				if (PKG.HasRawData())
				{
					for (auto& data : PKG.GetRawData())
					{
						// Append Key
						SingleSend(NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

						const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

						SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
						SingleSend(data.key(), strlen(data.key()) + 1, bPreviousSentFailed, sendToken);

						// Append Original Size
						/* Compression */
						//if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						//{
						//	const auto OriginalSizeStr = std::to_string(data.size() + std::to_string(data.size()).length());

						//	SingleSend(NET_RAW_DATA_ORIGINAL_SIZE, strlen(NET_RAW_DATA_ORIGINAL_SIZE), bPreviousSentFailed, sendToken);
						//	SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
						//	SingleSend(OriginalSizeStr.data(), OriginalSizeStr.length(), bPreviousSentFailed, sendToken);
						//	SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

						//	// now compress data
						//	//CompressData(data.value(), data.size());
						//}

						// Append Raw Data
						SingleSend(NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

						const auto rawDataLengthStr = std::to_string(data.size());

						SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
						SingleSend(data, bPreviousSentFailed, sendToken);

						data.set_free(false);
					}
				}

				SingleSend(NET_DATA, NET_DATA_LEN, bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
				SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed, sendToken);

				/* Append Packet Footer */
				SingleSend(NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN, bPreviousSentFailed, sendToken);
			}
			else
			{
				/* Compression */
				size_t original_dataBufferSize = dataBufferSize;
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					/* Compress Data */
					CompressData(dataBuffer.reference().get(), dataBufferSize);
				}

				combinedSize = dataBufferSize + NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + NET_DATA_LEN + NET_PACKET_FOOTER_LEN + 4;

				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					combinedSize += NET_UNCOMPRESSED_PACKET_SIZE_LEN;
					combinedSize += 2; // begin & end tag

					std::string original_dataBufferSize_str = std::to_string(original_dataBufferSize);
					combinedSize += original_dataBufferSize_str.length();
				}

				// Append Raw data packet size
				if (PKG.HasRawData()) combinedSize += PKG.GetRawDataFullSize();

				std::string dataSizeStr = std::to_string(dataBufferSize);
				combinedSize += dataSizeStr.length();

				const auto EntirePacketSizeStr = std::to_string(combinedSize + std::to_string(combinedSize).length());

				auto bPreviousSentFailed = false;

				/* Append Packet Header */
				SingleSend(NET_PACKET_HEADER, NET_PACKET_HEADER_LEN, bPreviousSentFailed, sendToken);

				// Append Packet Size Syntax
				SingleSend(NET_PACKET_SIZE, NET_PACKET_SIZE_LEN, bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
				SingleSend(EntirePacketSizeStr.data(), EntirePacketSizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

				/* Append Original Uncompressed Packet Size */
				/* Compression */
				if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
				{
					const auto UnCompressedPacketSizeStr = std::to_string(original_dataBufferSize + std::to_string(original_dataBufferSize).length());

					SingleSend(NET_UNCOMPRESSED_PACKET_SIZE, NET_UNCOMPRESSED_PACKET_SIZE_LEN, bPreviousSentFailed, sendToken);
					SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
					SingleSend(UnCompressedPacketSizeStr.data(), UnCompressedPacketSizeStr.length(), bPreviousSentFailed, sendToken);
					SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				}

				/* Append Packet Data */
				if (PKG.HasRawData())
				{
					for (auto& data : PKG.GetRawData())
					{
						// Append Key
						SingleSend(NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

						const auto KeyLengthStr = std::to_string(strlen(data.key()) + 1);

						SingleSend(KeyLengthStr.data(), KeyLengthStr.length(), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
						SingleSend(data.key(), strlen(data.key()) + 1, bPreviousSentFailed, sendToken);

						// Append Original Size
						/* Compression */
						//if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						//{
						//	const auto OriginalSizeStr = std::to_string(data.size() + std::to_string(data.size()).length());

						//	SingleSend(NET_RAW_DATA_ORIGINAL_SIZE, strlen(NET_RAW_DATA_ORIGINAL_SIZE), bPreviousSentFailed, sendToken);
						//	SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);
						//	SingleSend(OriginalSizeStr.data(), OriginalSizeStr.length(), bPreviousSentFailed, sendToken);
						//	SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);

						//	// now compress data
						//	//CompressData(data.value(), data.size());
						//}

						// Append Raw Data
						SingleSend(NET_RAW_DATA, strlen(NET_RAW_DATA), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_OPEN, 1, bPreviousSentFailed, sendToken);

						const auto rawDataLengthStr = std::to_string(data.size());

						SingleSend(rawDataLengthStr.data(), rawDataLengthStr.length(), bPreviousSentFailed, sendToken);
						SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
						SingleSend(data, bPreviousSentFailed, sendToken);

						data.set_free(false);
					}
				}

				SingleSend(NET_DATA, NET_DATA_LEN, bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_OPEN, strlen(NET_PACKET_BRACKET_OPEN), bPreviousSentFailed, sendToken);
				SingleSend(dataSizeStr.data(), dataSizeStr.length(), bPreviousSentFailed, sendToken);
				SingleSend(NET_PACKET_BRACKET_CLOSE, 1, bPreviousSentFailed, sendToken);
				SingleSend(dataBuffer, dataBufferSize, bPreviousSentFailed, sendToken);

				/* Append Packet Footer */
				SingleSend(NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN, bPreviousSentFailed, sendToken);
			}
		}

		/*
		*							Visualisation of packet structure in NET
		*	------------------------------------------------------------------------------------------
		*				CRYPTED VERSION					|		NON-CRYPTED VERSION
		*	------------------------------------------------------------------------------------------
		*	{KEYWORD}{SIZE}DATA
		*	------------------------------------------------------------------------------------------
		*	{BEGIN PACKET}								*		{BEGIN PACKET}
		*		{PACKET SIZE}{...}						*			{PACKET SIZE}{...}
		*			{KEY}{...}...						*						-
		*			{IV}{...}...						*						-
		*			{RAW DATA KEY}{...}...				*				{RAW DATA KEY}{...}...
		*			{RAW DATA}{...}...					*				{RAW DATA}{...}...
		*			{DATA}{...}...						*				{DATA}{...}...
		*	{END PACKET}								*		{END PACKET}
		*
		 */
		DWORD Client::DoReceive()
		{
			if (!IsConnected())
				return FREQUENZ;

			auto data_size = Ws2_32::recv(GetSocket(), reinterpret_cast<char*>(network.dataReceive), NET_OPT_DEFAULT_MAX_PACKET_SIZE, 0);
			if (data_size == SOCKET_ERROR)
			{
#ifdef BUILD_LINUX
				if (errno != EWOULDBLOCK)
#else
				if (Ws2_32::WSAGetLastError() != WSAEWOULDBLOCK)
#endif
				{
					memset(network.dataReceive, 0, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
					Disconnect();

#ifdef BUILD_LINUX
					if (ERRNO_ERROR_TRIGGERED) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(errno).c_str());
#else
					if (Ws2_32::WSAGetLastError() != 0) NET_LOG_PEER(CSTRING("%s"), Net::sock_err::getString(Ws2_32::WSAGetLastError()).c_str());
#endif

					return FREQUENZ;
				}

				ProcessPackets();
				memset(network.dataReceive, 0, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				return FREQUENZ;
			}

			// graceful disconnect
			if (data_size == 0)
			{
				memset(network.dataReceive, 0, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
				Disconnect();
				NET_LOG_PEER(CSTRING("Connection has been gracefully closed"));
				return FREQUENZ;
			}

			if (!network.data.valid())
			{
				network.AllocData(data_size);
				memcpy(network.data.get(), network.dataReceive, data_size);
				network.data.get()[data_size] = '\0';
				network.data_size = data_size;
			}
			else
			{
				if (network.data_full_size > 0
					&& network.data_size + data_size < network.data_full_size)
				{
					memcpy(&network.data.get()[network.data_size], network.dataReceive, data_size);
					network.data_size += data_size;
				}
				else
				{
					/* store incomming */
					const auto newBuffer = ALLOC<BYTE>(network.data_size + data_size + 1);
					memcpy(newBuffer, network.data.get(), network.data_size);
					memcpy(&newBuffer[network.data_size], network.dataReceive, data_size);
					newBuffer[network.data_size + data_size] = '\0';
					network.data.free();
					network.data = newBuffer; // pointer swap
					network.data_size += data_size;
				}
			}

			memset(network.dataReceive, 0, NET_OPT_DEFAULT_MAX_PACKET_SIZE);
			ProcessPackets();
			return 0;
		}

		bool Client::ValidHeader(bool& use_old_token)
		{
			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				// shift the first bytes to check if we are using the correct token - using old token
				for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
					network.data.get()[it] = network.data.get()[it] ^ network.lastToken;

				if (memcmp(&network.data.get()[0], NET_PACKET_HEADER, NET_PACKET_HEADER_LEN) != 0)
				{
					// shift back
					for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
						network.data.get()[it] = network.data.get()[it] ^ network.lastToken;

					// shift the first bytes to check if we are using the correct token - using cur token
					for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
						network.data.get()[it] = network.data.get()[it] ^ network.curToken;

					if (memcmp(&network.data.get()[0], NET_PACKET_HEADER, NET_PACKET_HEADER_LEN) != 0)
					{
						// shift back
						for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
							network.data.get()[it] = network.data.get()[it] ^ network.curToken;

						network.lastToken = network.curToken;
						network.curToken = Net::Coding::TOTP::generateToken(network.totp_secret, network.totp_secret_len, Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP ? network.curTime : time(nullptr), Isset(NET_OPT_TOTP_INTERVAL) ? (int)(GetOption<int>(NET_OPT_TOTP_INTERVAL) / 2) : (int)(NET_OPT_DEFAULT_TOTP_INTERVAL / 2));

						// shift the first bytes to check if we are using the correct token - using new token
						for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
							network.data.get()[it] = network.data.get()[it] ^ network.curToken;

						// [PROTOCOL] - check header is actually valid
						if (memcmp(&network.data.get()[0], NET_PACKET_HEADER, NET_PACKET_HEADER_LEN) != 0)
						{
							network.clear();
							Disconnect();
							NET_LOG_ERROR(CSTRING("[NET] - Received a frame with an invalid header"));
							return false;
						}

						// sift back using new token
						for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
							network.data.get()[it] = network.data.get()[it] ^ network.curToken;

						use_old_token = false;
					}
					else
					{
						// sift back using cur token
						for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
							network.data.get()[it] = network.data.get()[it] ^ network.curToken;

						use_old_token = false;
					}
				}
				else
				{
					// sift back using old token
					for (size_t it = 0; it < NET_PACKET_HEADER_LEN; ++it)
						network.data.get()[it] = network.data.get()[it] ^ network.lastToken;
				}
			}
			else
			{
				// [PROTOCOL] - check header is actually valid
				if (memcmp(&network.data.get()[0], NET_PACKET_HEADER, NET_PACKET_HEADER_LEN) != 0)
				{
					network.clear();
					Disconnect();
					NET_LOG_ERROR(CSTRING("[NET] - Received a frame with an invalid header"));
					return false;
				}
			}

			return true;
		}

		void Client::ProcessPackets()
		{
			// check valid data size
			if (!network.data_size)
				return;

			if (network.data_size == INVALID_SIZE)
				return;

			if (network.data_size < NET_PACKET_HEADER_LEN) return;

			auto use_old_token = true;
			bool already_checked = false;

			// [PROTOCOL] - read data full size from header
			if (!network.data_full_size || network.data_full_size == INVALID_SIZE)
			{
				already_checked = true;
				if (!ValidHeader(use_old_token)) return;

				// read entire packet size
				const size_t start = NET_PACKET_HEADER_LEN + NET_PACKET_SIZE_LEN + 1;
				for (size_t i = start; i < network.data_size; ++i)
				{
					// shift the bytes
					if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
						network.data.get()[i] = network.data.get()[i] ^ (use_old_token ? network.lastToken : network.curToken);

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

							// shift all the way back
							if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
							{
								for (size_t it = start; it < i + 1; ++it)
									network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
							}

							return;
						}

						// shift all the way back
						if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
						{
							for (size_t it = start; it < i + 1; ++it)
								network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
						}

						break;
					}
				}
			}

			/* Compression */
			if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
			{
				const size_t start = network.data_offset + NET_UNCOMPRESSED_PACKET_SIZE_LEN + 2;
				for (size_t i = start; i < network.data_size; ++i)
				{
					// iterate until we have found the end tag
					if (!memcmp(&network.data.get()[i], NET_PACKET_BRACKET_CLOSE, 1))
					{
						network.data_offset = i;
						const auto size = i - start;
						char* end = (char*)network.data.get()[start] + size;
						network.data_original_uncompressed_size = strtoull((const char*)&network.data.get()[start], &end, 10);

						break;
					}
				}
			}

			// keep going until we have received the entire packet
			if (!network.data_full_size || network.data_full_size == INVALID_SIZE || network.data_size < network.data_full_size) return;

			if (!already_checked)
				if (!ValidHeader(use_old_token)) return;

			// shift only as much as required
			if (Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP)
			{
				for (size_t it = 0; it < network.data_full_size; ++it)
					network.data.get()[it] = network.data.get()[it] ^ (use_old_token ? network.lastToken : network.curToken);
			}

			// [PROTOCOL] - check footer is actually valid
			if (memcmp(&network.data.get()[network.data_full_size - NET_PACKET_FOOTER_LEN], NET_PACKET_FOOTER, NET_PACKET_FOOTER_LEN) != 0)
			{
				network.clear();
				Disconnect();
				NET_LOG_ERROR(CSTRING("[NET] - Received a frame with an invalid footer"));
				return;
			}

			// Execute the packet
			ExecutePacket();

			// re-alloc buffer
			const auto leftSize = static_cast<int>(network.data_size - network.data_full_size) > 0 ? network.data_size - network.data_full_size : INVALID_SIZE;
			if (leftSize != INVALID_SIZE
				&& leftSize > 0)
			{
				const auto leftBuffer = ALLOC<BYTE>(leftSize + 1);
				memcpy(leftBuffer, &network.data.get()[network.data_full_size], leftSize);
				leftBuffer[leftSize] = '\0';
				network.clearData();
				network.data = leftBuffer; // swap pointer
				network.data_size = leftSize;
				return;
			}

			network.clearData();
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
			if (!tpe)
			{
				return 1;
			}

			if (!tpe->m_client->CheckDataN(tpe->m_packetId, *tpe->m_packet))
				if (!tpe->m_client->CheckData(tpe->m_packetId, *tpe->m_packet))
				{
					tpe->m_client->Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Frame is not defined"));
				}

			/* hence we had to create a copy to work with this data in seperate thread, we also have to handle the deletion of this block */
			if (tpe->m_packet->HasRawData())
			{
				std::vector<Net::RawData_t>& rawData = tpe->m_packet->GetRawData();
				for (auto& data : rawData)
				{
					data.free();
				}
			}

			FREE<Net::Packet>(tpe->m_packet);
			FREE<TPacketExcecute>(tpe);
			return 0;
		}

		void Client::ExecutePacket()
		{
			NET_CPOINTER<BYTE> data;
			NET_CPOINTER<Net::Packet> pPacket(ALLOC<Net::Packet>());
			if (!pPacket.valid())
			{
				Disconnect();
				return;
			}

			/* Crypt */
			if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && network.RSAHandshake)
			{
				auto offset = network.data_offset + 1;

				NET_CPOINTER<BYTE> AESKey;
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

				NET_CPOINTER<BYTE> AESIV;
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
					pPacket.free();
					AESKey.free();
					AESIV.free();
					Disconnect();
					NET_LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-Key & RSA and Base64"));
					return;
				}

				if (!network.RSA.decryptBase64(AESIV.reference().get(), AESIVSize))
				{
					pPacket.free();
					AESKey.free();
					AESIV.free();
					Disconnect();
					NET_LOG_ERROR(CSTRING("[NET] - Failure on decrypting frame using AES-IV & RSA and Base64"));
					return;
				}

				NET_AES aes;
				if (!aes.init(reinterpret_cast<const char*>(AESKey.get()), reinterpret_cast<const char*>(AESIV.get())))
				{
					pPacket.free();
					AESKey.free();
					AESIV.free();
					Disconnect();
					NET_LOG_ERROR(CSTRING("[NET] - Initializing AES failure"));
					return;
				}

				AESKey.free();
				AESIV.free();

				do
				{
					// look for raw data tag
					if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
					{
						offset += strlen(NET_RAW_DATA_KEY);

						// read size
						NET_CPOINTER<BYTE> key;
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

						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
						{
							offset += strlen(NET_RAW_DATA);

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

							///* Decompression */
							//if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
							//{
							//	DecompressData(entry.value(), entry.value(), entry.size(), true);
							//	entry.set_free(true);
							//}

							/* decrypt aes */
							if (!aes.decrypt(entry.value(), entry.size()))
							{
								pPacket.free();
								Disconnect();
								NET_LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));
								return;
							}

							/* in seperate thread we need to create a copy of this data-set */
							if (Isset(NET_OPT_EXECUTE_PACKET_ASYNC) ? GetOption<bool>(NET_OPT_EXECUTE_PACKET_ASYNC) : NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC)
							{
								BYTE* copy = ALLOC<BYTE>(entry.size());
								memcpy(copy, entry.value(), entry.size());
								entry.set(copy);
							}

							pPacket.get()->AddRaw(entry);
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
						if (!aes.decrypt(data.get(), packetSize))
						{
							pPacket.free();
							data.free();
							Disconnect();
							NET_LOG_PEER(CSTRING("[NET] - Decrypting frame has been failed"));
							return;
						}

						/* Decompression */
						if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						{
							DecompressData(data.reference().get(), packetSize, network.data_original_uncompressed_size);
						}
					}

					// we have reached the end of reading
					if (offset + NET_PACKET_FOOTER_LEN >= network.data_full_size)
						break;

				} while (true);
			}
			else
			{
				auto offset = network.data_offset + 1;

				do
				{
					// look for raw data tag
					if (!memcmp(&network.data.get()[offset], NET_RAW_DATA_KEY, strlen(NET_RAW_DATA_KEY)))
					{
						offset += strlen(NET_RAW_DATA_KEY);

						// read size
						NET_CPOINTER<BYTE> key;
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

						if (!memcmp(&network.data.get()[offset], NET_RAW_DATA, strlen(NET_RAW_DATA)))
						{
							offset += strlen(NET_RAW_DATA);

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

							///* Decompression */
							//if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
							//{
							//	DecompressData(entry.value(), entry.value(), entry.size(), true);
							//	entry.set_free(true);
							//}

							/* in seperate thread we need to create a copy of this data-set */
							if (Isset(NET_OPT_EXECUTE_PACKET_ASYNC) ? GetOption<bool>(NET_OPT_EXECUTE_PACKET_ASYNC) : NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC)
							{
								BYTE* copy = ALLOC<BYTE>(entry.size());
								memcpy(copy, entry.value(), entry.size());
								entry.set(copy);
							}

							pPacket.get()->AddRaw(entry);
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

						/* Decompression */
						if (Isset(NET_OPT_USE_COMPRESSION) ? GetOption<bool>(NET_OPT_USE_COMPRESSION) : NET_OPT_DEFAULT_USE_COMPRESSION)
						{
							DecompressData(data.reference().get(), packetSize, network.data_original_uncompressed_size);
						}
					}

					// we have reached the end of reading
					if (offset + NET_PACKET_FOOTER_LEN >= network.data_full_size)
						break;

				} while (true);
			}

			if (!data.valid())
			{
				pPacket.free();
				Disconnect();
				NET_LOG_PEER(CSTRING("[NET] - JSON data is not valid"));
				return;
			}
			
			int packetId = -1;
			{
				Net::Json::Document doc;
				if (!doc.Deserialize(reinterpret_cast<char*>(data.get())))
				{
					pPacket.free();
					data.free();
					Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Unable to deserialize json data"));
					return;
				}

				data.free();

				if (!(doc[CSTRING("ID")] && doc[CSTRING("ID")]->is_int()))
				{
					pPacket.free();
					Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));
					return;
				}

				packetId = doc[CSTRING("ID")]->as_int();
				if (packetId < 0)
				{
					pPacket.free();
					Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Frame identification is not valid"));
					return;
				}

				if (!(doc[CSTRING("CONTENT")] && doc[CSTRING("CONTENT")]->is_object())
					&& !(doc[CSTRING("CONTENT")] && doc[CSTRING("CONTENT")]->is_array()))
				{
					pPacket.free();
					Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Frame is empty"));
					return;
				}

				if (doc[CSTRING("CONTENT")]->is_object())
				{
					pPacket.get()->Data().Set(doc[CSTRING("CONTENT")]->as_object());
				}
				else if (doc[CSTRING("CONTENT")]->is_array())
				{
					pPacket.get()->Data().Set(doc[CSTRING("CONTENT")]->as_array());
				}
			}

			/*
			* check for option async to execute the callback in a seperate thread
			*/
			if (Isset(NET_OPT_EXECUTE_PACKET_ASYNC) ? GetOption<bool>(NET_OPT_EXECUTE_PACKET_ASYNC) : NET_OPT_DEFAULT_EXECUTE_PACKET_ASYNC)
			{
				TPacketExcecute* tpe = ALLOC<TPacketExcecute>();
				tpe->m_packet = pPacket.get();
				tpe->m_client = this;
				tpe->m_packetId = packetId;
				if (Net::Thread::Create(ThreadPacketExecute, tpe))
				{
					return;
				}

				FREE<TPacketExcecute>(tpe);
			}

			/*
			* execute in current thread
			*/
			if (!CheckDataN(packetId, *pPacket.ref().get()))
				if (!CheckData(packetId, *pPacket.ref().get()))
				{
					Disconnect();
					NET_LOG_PEER(CSTRING("[NET] - Frame is not defined"));
				}

			pPacket.free();
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
		}

		void Client::CompressData(BYTE*& data, BYTE*& out, size_t& size, const bool skip_free)
		{
#ifdef DEBUG
			const auto PrevSize = size;
#endif

			BYTE* m_pCompressed = 0;
			size_t m_iCompressedLen = 0;
			NET_ZLIB::Compress(data, size, m_pCompressed, m_iCompressedLen);

			if(!skip_free)
			{
				FREE<BYTE>(data);
			}

			out = m_pCompressed;
			size = m_iCompressedLen;

#ifdef DEBUG
			NET_LOG_DEBUG(CSTRING("[NET] - Compressed data from size %llu to %llu"), PrevSize, size);
#endif
		}

		void Client::DecompressData(BYTE*& data, size_t& size, size_t original_size)
		{
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

		bool Client::CreateTOTPSecret()
		{
			if (!(Isset(NET_OPT_USE_TOTP) ? GetOption<bool>(NET_OPT_USE_TOTP) : NET_OPT_DEFAULT_USE_TOTP))
				return false;

			network.curTime = time(nullptr);
			if (Isset(NET_OPT_USE_NTP) ? GetOption<bool>(NET_OPT_USE_NTP) : NET_OPT_DEFAULT_USE_NTP)
			{
				auto time = Net::Protocol::NTP::Exec(Isset(NET_OPT_NTP_HOST) ? GetOption<char*>(NET_OPT_NTP_HOST) : NET_OPT_DEFAULT_NTP_HOST,
					Isset(NET_OPT_NTP_PORT) ? GetOption<u_short>(NET_OPT_NTP_PORT) : NET_OPT_DEFAULT_NTP_PORT);

				if (!time.valid())
				{
					NET_LOG_ERROR(CSTRING("[NET] - critical failure on calling NTP host"));
					return false;
				}

				network.curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
			}

#ifdef BUILD_LINUX
			struct tm* tm = nullptr;
			tm = gmtime(&network.curTime);
			tm->tm_hour = Net::Util::roundUp(tm->tm_hour, 10);
			tm->tm_min = Net::Util::roundUp(tm->tm_min, 10);
			tm->tm_sec = 0;
			const auto txTm = mktime(tm);
#else
			tm tm;
			gmtime_s(&tm, &network.curTime);
			tm.tm_hour = Net::Util::roundUp(tm.tm_hour, 10);
			tm.tm_min = Net::Util::roundUp(tm.tm_min, 10);
			tm.tm_sec = 0;
			const auto txTm = mktime(&tm);
#endif

			const auto strTime = ctime(&txTm);
			network.totp_secret_len = strlen(strTime);

			FREE<byte>(network.totp_secret);
			network.totp_secret = ALLOC<byte>(network.totp_secret_len + 1);
			memcpy(network.totp_secret, strTime, network.totp_secret_len);
			network.totp_secret[network.totp_secret_len] = '\0';
			Net::Coding::Base32::encode(network.totp_secret, network.totp_secret_len);

			network.curToken = 0;
			network.lastToken = 0;

			return true;
		}

		NET_NATIVE_PACKET_DEFINITION_BEGIN(Client);
		NET_DEFINE_PACKET(RSAHandshake, NET_NATIVE_PACKET_ID::PKG_RSAHandshake);
		NET_DEFINE_PACKET(Version, NET_NATIVE_PACKET_ID::PKG_Version);
		NET_DEFINE_PACKET(EstabilishConnection, NET_NATIVE_PACKET_ID::PKG_Estabilish);
		NET_DEFINE_PACKET(Close, NET_NATIVE_PACKET_ID::PKG_Close);
		NET_PACKET_DEFINITION_END;

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

		if (!(pkg[CSTRING("PublicKey")] && pkg[CSTRING("PublicKey")]->is_string()))
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a handshake frame, received public key is not valid, rejecting the frame"), FUNCTION_NAME);
			return;
		}

		// send our generated Public Key to the Server
		NET_PACKET reply;
		auto MyPublicKey = network.RSA.publicKey();

		size_t b64len = MyPublicKey.size();
		BYTE* b64 = ALLOC<BYTE>(b64len + 1);
		memcpy(b64, MyPublicKey.data(), b64len);
		b64[b64len] = 0;

		Net::Coding::Base64::encode(b64, b64len);

		reply[CSTRING("PublicKey")] = reinterpret_cast<char*>(b64);
		NET_SEND(NET_NATIVE_PACKET_ID::PKG_RSAHandshake, reply);

		FREE<byte>(b64);

		// from now we use the Cryption, synced with Server
		{
			b64len = strlen(pkg[CSTRING("PublicKey")]->as_string());
			b64 = ALLOC<BYTE>(b64len + 1);
			memcpy(b64, pkg[CSTRING("PublicKey")]->as_string(), b64len);
			b64[b64len] = 0;

			Net::Coding::Base64::decode(b64, b64len);

			network.RSA.setPublicKey(reinterpret_cast<char*>(b64));
			network.RSAHandshake = true;
		}
		NET_END_PACKET;

		NET_BEGIN_PACKET(Client, Version);
		if ((Isset(NET_OPT_USE_CIPHER) ? GetOption<bool>(NET_OPT_USE_CIPHER) : NET_OPT_DEFAULT_USE_CIPHER) && !network.RSAHandshake)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a version frame, client has not performed a handshake yet, rejecting the frame"), FUNCTION_NAME);
			return;
		}
		if (network.estabilished)
		{
			Disconnect();
			NET_LOG_ERROR(CSTRING("[NET][%s] - received a version frame, client has already been estabilished, rejecting the frame"), FUNCTION_NAME);
			return;
		}

		NET_PACKET reply;
		reply[CSTRING("MajorVersion")] = Version::Major();
		reply[CSTRING("MinorVersion")] = Version::Minor();
		reply[CSTRING("Revision")] = Version::Revision();
		const auto Key = Version::Key().get();
		reply[CSTRING("Key")] = Key.get();
		NET_SEND(NET_NATIVE_PACKET_ID::PKG_Version, reply);
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
		ConnectionClosed();

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
			OnVersionMismatch();

		// Callback
		OnConnectionClosed(code);
		NET_END_PACKET;
	}
}
