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

#define DEFAULT_MAX_PEER_COUNT 4

#include <Net/Net/NetPeerPool.h>
#include <Net/assets/manager/logmanager.h>

Net::PeerPool::peerInfo_t::peerInfo_t()
{
	this->SetPeer(nullptr);
	this->SetWorker(nullptr);
	this->SetCallbackOnDelete(nullptr);
}

Net::PeerPool::peerInfo_t::peerInfo_t(void* peer)
{
	this->SetPeer(peer);
	this->SetWorker(nullptr);
	this->SetCallbackOnDelete(nullptr);
}

Net::PeerPool::peerInfo_t::peerInfo_t(void* peer, WorkStatus_t(*fncWork)(void* peer))
{
	this->SetPeer(peer);
	this->SetWorker(fncWork);
	this->SetCallbackOnDelete(nullptr);
}

Net::PeerPool::peerInfo_t::peerInfo_t(void* peer, void (*fncCallbackOnDelete)(void* peer))
{
	this->SetPeer(peer);
	this->SetWorker(nullptr);
	this->SetCallbackOnDelete(fncCallbackOnDelete);
}

Net::PeerPool::peerInfo_t::peerInfo_t(void* peer, WorkStatus_t(*fncWork)(void* peer), void (*fncCallbackOnDelete)(void* peer))
{
	this->SetPeer(peer);
	this->SetWorker(fncWork);
	this->SetCallbackOnDelete(fncCallbackOnDelete);
}

void Net::PeerPool::peerInfo_t::SetPeer(void* peer)
{
	this->peer = peer;
}

void* Net::PeerPool::peerInfo_t::GetPeer()
{
	return this->peer;
}

void Net::PeerPool::peerInfo_t::SetWorker(WorkStatus_t(*fncWork)(void* peer))
{
	this->fncWork = fncWork;
}

void* Net::PeerPool::peerInfo_t::GetWorker()
{
	if (this->fncWork) return (void*)this->fncWork;
	return nullptr;
}

void Net::PeerPool::peerInfo_t::SetCallbackOnDelete(void (*fncCallbackOnDelete)(void* peer))
{
	this->fncCallbackOnDelete = fncCallbackOnDelete;
}

void* Net::PeerPool::peerInfo_t::GetCallbackOnDelete()
{
	if (this->fncCallbackOnDelete) return (void*)this->fncCallbackOnDelete;
	return nullptr;
}

Net::PeerPool::PeerPool_t::PeerPool_t()
{
	fncSleep = nullptr;
	ms_sleep_time = 100;
	max_peers = DEFAULT_MAX_PEER_COUNT;
}

void Net::PeerPool::PeerPool_t::set_sleep_time(DWORD ms_sleep_time)
{
	this->ms_sleep_time = ms_sleep_time;
}

DWORD Net::PeerPool::PeerPool_t::get_sleep_time()
{
	return this->ms_sleep_time;
}

void Net::PeerPool::PeerPool_t::set_sleep_function(void (*fncSleep)(DWORD time))
{
	this->fncSleep = fncSleep;
}

void* Net::PeerPool::PeerPool_t::get_sleep_function()
{
	return (void*)this->fncSleep;
}

void Net::PeerPool::PeerPool_t::set_max_peers(size_t max_peers)
{
	this->max_peers = max_peers;
}

size_t Net::PeerPool::PeerPool_t::get_max_peers()
{
	return this->max_peers;
}

bool Net::PeerPool::PeerPool_t::check_more_threads_needed()
{
	for (const auto& pool : peer_threadpool)
	{
		if (count_peers(pool) != max_peers)
		{
			return false;
		}
	}

	return true;
}

void Net::PeerPool::PeerPool_t::threapool_push(peer_threadpool_t* pool)
{
	peer_threadpool.emplace_back(pool);
}

struct threadpool_manager_data_t
{
	Net::PeerPool::PeerPool_t* pClass;
	Net::PeerPool::peer_threadpool_t* pool;
};

NET_THREAD(threadpool_manager)
{
	auto data = (threadpool_manager_data_t*)parameter;
	auto pClass = data->pClass;
	auto pool = data->pool;

	while (data)
	{
		BYTE take_rest = 1;

		for (size_t i = 0; i < pClass->get_max_peers(); ++i)
		{
			auto& peer = pool->vPeers[i];
			if (peer == nullptr)
			{
				auto waiting_peer = pClass->queue_pop();
				if (waiting_peer)
				{
					peer = waiting_peer;
				}

				// process in next iteration
				continue;
			}

			// Automaticly set to stop if no worker function is set
			Net::PeerPool::WorkStatus_t ret = Net::PeerPool::WorkStatus_t::STOP;

			auto fncWorkPointer = peer->GetWorker();
			if (fncWorkPointer)
			{
				auto fncWork = reinterpret_cast<Net::PeerPool::WorkStatus_t(*)(void* peer)>(fncWorkPointer);
				ret = (*fncWork)(peer->GetPeer());
			}

			switch (ret)
			{
			case Net::PeerPool::WorkStatus_t::STOP:
			{
				auto fncCallbackOnDeletePointer = peer->GetCallbackOnDelete();
				if (fncCallbackOnDeletePointer)
				{
					auto fncCallbackOnDelete = reinterpret_cast<void (*)(void* peer)>(fncCallbackOnDeletePointer);
					(*fncCallbackOnDelete)(peer->GetPeer());
				}

				peer = nullptr;
				break;
			}

			case Net::PeerPool::WorkStatus_t::CONTINUE:
			{
				/* move peers into available threads to reduce the amount of running threads */
				const std::lock_guard<std::recursive_mutex> lock(*pClass->get_peer_threadpool_mutex());
				const auto p = pClass->threadpool_get_free_slot_in_target_pool(pool);
				if (p)
				{
					// move peer pointer
					for (size_t j = 0; j < pClass->get_max_peers(); ++j)
					{
						auto& target_peer = p->vPeers[j];
						if (target_peer == nullptr)
						{
							target_peer = peer;
							peer = nullptr;
							break;
						}
					}
				}
				break;
			}

			// do not take a rest if we want to forward on processing worker function rapidly
			case Net::PeerPool::WorkStatus_t::FORWARD:
			{
				take_rest = 0;
				break;
			}

			default:
				break;
			}
		}

		// close this thread
		if (pClass->count_peers(pool) == 0)
		{
			// erase from vector
			const std::lock_guard<std::recursive_mutex> lock(*pClass->get_peer_threadpool_mutex());
			for (auto it = pClass->get_peer_threadpool().begin(); it != pClass->get_peer_threadpool().end(); it++)
			{
				auto p = *it;

				// compare mem address
				if (p == pool)
				{
					pClass->get_peer_threadpool().erase(it);
					break;
				}
			}

			FREE<Net::PeerPool::peerInfo_t>(pool->vPeers);
			FREE<threadpool_manager_data_t>(data);
			return NULL;
		}

		if (take_rest == 1)
		{
			auto fncSleepPointer = pClass->get_sleep_function();
			if (fncSleepPointer)
			{
				auto fncSleep = reinterpret_cast<void (*)(DWORD time)>(fncSleepPointer);
				(*fncSleep)(pClass->get_sleep_time());
			}
		}
	}

	FREE<threadpool_manager_data_t>(data);
	return NULL;
}

void Net::PeerPool::PeerPool_t::threadpool_add()
{
	peer_threadpool_t* pool = ALLOC<peer_threadpool_t>();
	if (pool == nullptr)
	{
		return;
	}

	pool->vPeers = ALLOC<Net::PeerPool::peerInfo_t*>(get_max_peers());
	if (pool->vPeers == nullptr)
	{
		FREE<Net::PeerPool::peer_threadpool_t>(pool);
		return;
	}

	for (size_t i = 0; i < get_max_peers(); ++i)
	{
		pool->vPeers[i] = nullptr;
	}

	// dispatch the thread
	auto data = ALLOC<threadpool_manager_data_t>();
	data->pClass = this;
	data->pool = pool;
	const auto hThread = Net::Thread::Create(threadpool_manager, reinterpret_cast<void*>(data));
	if (hThread)
	{
		// Close only closes handle, it does not close the thread
		Net::Thread::Close(hThread);
	}

	// push to vector
	threapool_push(pool);
}

Net::PeerPool::peer_threadpool_t* Net::PeerPool::PeerPool_t::threadpool_get_free_slot_in_target_pool(peer_threadpool_t* current_pool)
{
	if (peer_queue.empty() == 0)
	{
		return nullptr;
	}

	for (const auto& pool : peer_threadpool)
	{
		if (pool == current_pool)
		{
			continue;
		}

		for (size_t i = 0; i < get_max_peers(); ++i)
		{
			auto& peer = pool->vPeers[i];
			if (peer == nullptr)
			{
				return pool;
			}
		}
	}

	return nullptr;
}

Net::PeerPool::peerInfo_t* Net::PeerPool::PeerPool_t::queue_pop()
{
	if (peer_queue.empty())
	{
		return nullptr;
	}

	auto peer = peer_queue.back();
	peer_queue.pop_back();
	return peer;
}

std::vector<Net::PeerPool::peer_threadpool_t*>& Net::PeerPool::PeerPool_t::get_peer_threadpool()
{
	return this->peer_threadpool;
}

std::recursive_mutex* Net::PeerPool::PeerPool_t::get_peer_threadpool_mutex()
{
	return &this->peer_threadpool_mutex;
}

void Net::PeerPool::PeerPool_t::add(peerInfo_t info)
{
	peer_queue.emplace_back(ALLOC<peerInfo_t, peerInfo_t>(1, info));

	if (check_more_threads_needed())
	{
		this->threadpool_add();
	}
}

void Net::PeerPool::PeerPool_t::add(peerInfo_t* info)
{
	peer_queue.emplace_back(info);

	if (check_more_threads_needed())
	{
		this->threadpool_add();
	}
}

size_t Net::PeerPool::PeerPool_t::count_peers_all()
{
	size_t peers = 0;

	for (const auto pool : peer_threadpool)
	{
		for (size_t i = 0; i < get_max_peers(); ++i)
		{
			auto peer = pool->vPeers[i];
			if (peer)
			{
				peers++;
			}
		}
	}

	return peers;
}

size_t Net::PeerPool::PeerPool_t::count_peers(peer_threadpool_t* pool)
{
	size_t peers = 0;

	for (size_t i = 0; i < get_max_peers(); ++i)
	{
		auto peer = pool->vPeers[i];
		if (peer)
		{
			peers++;
		}
	}

	return peers;
}

size_t Net::PeerPool::PeerPool_t::count_pools()
{
	return peer_threadpool.size();
}
