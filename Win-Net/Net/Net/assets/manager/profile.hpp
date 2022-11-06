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
			void* peer;

		public:
			Net::Json::Document data;
			void* ext;

			Profile_t()
			{
				peer = nullptr;
				ext = nullptr;
			}

			Profile_t(void* peer)
			{
				this->peer = peer;
				ext = nullptr;
			}

			bool valid()
			{
				return (peer != nullptr);
			}

			void clear()
			{
				peer = nullptr;
				FREE<void>(ext);
			}

			template <typename T>
			T Peer()
			{
				return (T)peer;
			}
		};

		class Profile
		{
			NET_PROFILE_DATA* data;
			size_t max_entries;
			size_t c_entries;
			std::recursive_mutex critical;

			size_t get_free_slot()
			{
				if (!data) return INVALID_SIZE;

				std::lock_guard<std::recursive_mutex> guard(critical);

				for (size_t i = 0; i < max_entries; ++i)
					if (data[i].Peer<void*>() == nullptr) return i;

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
				if (!data) return nullptr;

				auto slot = get_free_slot();
				if (slot == INVALID_SIZE) return nullptr;
				data[slot] = NET_PROFILE_DATA(peer);
				++c_entries;
				return this->peer(peer);
			}

			void Remove(void* peer)
			{
				if (!data) return;

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
				if (!data) return nullptr;

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
						return data[i].ext;
				}

				return nullptr;
			}

			void setPeerExt(void* peer, void* ext)
			{
				if (!data) return;

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
					{
						data[i].ext = ext;
						break;
					}
				}
			}

			Net::Json::Document* peer(void* peer)
			{
				if (!data) return nullptr;

				for (size_t i = 0; i < max_entries; ++i)
				{
					if (data[i].Peer<void*>() == peer)
						return &data[i].data;
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