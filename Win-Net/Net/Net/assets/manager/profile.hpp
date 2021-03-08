#pragma once
#include <Net/Net/Net.h>
#include <Net/Net/Package.h>
#include <mutex>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
template <typename T>
struct NetProfile_t
{
	T peer;
	Package* data;

	NetProfile_t(T peer)
	{
		this->peer = peer;
		data = new Package();
	}

	void clear()
	{
		FREE(data);
		peer = NULL;
	}
};

template <typename T>
class NetProfile
{
	std::vector<NetProfile_t<T>> info;
	std::mutex critical;

public:
	Package* Add(T peer)
	{
		critical.lock();
		info.emplace_back(NetProfile_t<T>(peer));
		critical.unlock();
		return this->peer(peer);
	}

	void Remove(T peer)
	{
		critical.lock();
		for (auto it = info.begin(); it != info.end(); ++it)
		{
			NetProfile_t<T> entry = (NetProfile_t<T>) * it;
			if (entry.peer == peer)
			{
				entry.clear();
				info.erase(it);
				break;
			}
		}
		critical.unlock();
	}

	Package* peer(T peer)
	{
		critical.lock();
		for (NetProfile_t<T> it : info)
		{
			if (it.peer == peer)
			{
				critical.unlock();
				return it.data;
			}
		}
		critical.unlock();
		return nullptr;
	}

	std::vector<NetProfile_t<T>>& all()
	{
		return info;
	}

	size_t count() const
	{
		return info.size();
	}
};
NET_NAMESPACE_END
NET_DSA_END