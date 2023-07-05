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
#define NET_PROFILE Net::Manager::Profile
#define NET_PROFILE_DATA Net::Manager::Profile_t

#include <Net/Net/Net.h>
#include <Net/Net/NetJson.h>
#include <mutex>

NET_DSA_BEGIN
namespace Net
{
	namespace Manager
	{
		class Profile_t
		{
			void* m_peer;

		public:
			Net::Json::Document m_doc;
			void* m_ext;

			Profile_t()
			{
				m_peer = nullptr;
				m_ext = nullptr;
			}

			Profile_t(void* peer)
			{
				m_peer = peer;
				m_ext = nullptr;
			}

			bool valid()
			{
				return (m_peer != nullptr);
			}

			void clear()
			{
				m_peer = nullptr;
				FREE<byte>(m_ext);
			}

			template <typename T>
			T Peer()
			{
				return (T)m_peer;
			}
		};

		class Profile
		{
			NET_PROFILE_DATA* data;
			size_t max_entries;
			size_t c_entries;

			size_t get_free_slot()
			{
				if (data == nullptr)
				{
					return INVALID_SIZE;
				}

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == nullptr) 
					{
						return i;
					}
				}

				// indicates all slots are being in use
				return INVALID_SIZE;
			}

		public:
			Profile()
			{
				data = nullptr;
				max_entries = 0;
				c_entries = 0;
			}

			void Init(size_t max_entries)
			{
				this->max_entries = max_entries;
				data = ALLOC<NET_PROFILE_DATA>(this->max_entries);
				memset(data, 0, this->max_entries);
			}

			~Profile()
			{
				FREE<Net::Manager::Profile_t>(data);
				max_entries = 0;
			}

			Net::Json::Document* Add(void* peer)
			{
				if (data == nullptr) 
				{
					return nullptr;
				}

				auto slot = get_free_slot();
				if (slot == INVALID_SIZE)
				{
					return nullptr;
				}

				NET_PROFILE_DATA profile(peer);
				data[slot] = profile;
				++c_entries;

				return this->peer(peer);
			}

			void Remove(void* peer)
			{
				if (data == nullptr)
				{
					return;
				}

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
					{
						data[i].clear();
						--c_entries;
						break;
					}
				}
			}

			void* peerExt(void* peer)
			{
				if (data == nullptr)
				{
					return nullptr;
				}

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
					{
						return data[i].m_ext;
					}
				}

				return nullptr;
			}

			void setPeerExt(void* peer, void* ext)
			{
				if (data == nullptr)
				{
					return;
				}

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
					{
						data[i].m_ext = ext;
						break;
					}
				}
			}

			Net::Json::Document* peer(void* peer)
			{
				if (data == nullptr)
				{
					return nullptr;
				}

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
					{
						return &data[i].m_doc;
					}
				}

				return nullptr;
			}

			NET_PROFILE_DATA* all()
			{
				return data;
			}

			size_t count() const
			{
				return max_entries;
			}

			size_t count_entries() const
			{
				return c_entries;
			}
		};
	}
}
NET_DSA_END
