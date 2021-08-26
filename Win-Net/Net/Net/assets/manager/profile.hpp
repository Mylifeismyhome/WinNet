#pragma once
#define NET_PROFILE Net::Manager::Profile
#define NET_PROFILE_DATA Net::Manager::Profile_t

#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <mutex>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Manager)
template <typename T>
struct Profile_t
{
	T peer;
	Package* data;
	void* ext;

	Profile_t(T peer)
	{
		this->peer = peer;
		data = new Package();
		ext = nullptr;
	}

	void clear()
	{
		FREE(data);
		FREE(ext);
		peer = NULL;
	}
};

template <typename T>
class Profile
{
	std::vector<NET_PROFILE_DATA<T>> info;
	std::recursive_mutex critical;

public:
	Package* Add(T peer)
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		info.emplace_back(NET_PROFILE_DATA<T>(peer));
		return this->peer(peer);
	}

	void Remove(T peer)
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		for (auto it = info.begin(); it != info.end(); ++it)
		{
			NET_PROFILE_DATA<T> entry = (NET_PROFILE_DATA<T>) * it;
			if (entry.peer == peer)
			{
				entry.clear();
				info.erase(it);
				break;
			}
		}
	}

	void* peerExt(T peer)
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		for (NET_PROFILE_DATA<T>& it : info)
		{
			if (it.peer == peer)
				return it.ext;
		}
		return nullptr;
	}

	void setPeerExt(T peer, void* ext)
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		for (NET_PROFILE_DATA<T>& it : info)
		{
			if (it.peer == peer)
			{
				it.ext = ext;
				break;
			}
		}
	}

	Package* peer(T peer)
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		for (NET_PROFILE_DATA<T>& it : info)
		{
			if (it.peer == peer)
				return it.data;
		}
		return nullptr;
	}

	std::vector<NET_PROFILE_DATA<T>>& All()
	{
		std::lock_guard<std::recursive_mutex> guard(critical);
		return info;
	}

	size_t count() const
	{
		return info.size();
	}
};
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_DSA_END