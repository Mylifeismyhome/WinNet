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
	if (this->fncWork) return this->fncWork;
	return nullptr;
}

void Net::PeerPool::peerInfo_t::SetCallbackOnDelete(void (*fncCallbackOnDelete)(void* peer))
{
	this->fncCallbackOnDelete = fncCallbackOnDelete;
}

void* Net::PeerPool::peerInfo_t::GetCallbackOnDelete()
{
	if (this->fncCallbackOnDelete) return this->fncCallbackOnDelete;
	return nullptr;
}

Net::PeerPool::PeerPool_t::PeerPool_t()
{
	peer_mutex = new std::mutex();
	peer_threadpool_mutex = new std::mutex();

	fncSleep = nullptr;
	ms_sleep_time = 100;
	max_peers = DEFAULT_MAX_PEER_COUNT;
}

Net::PeerPool::PeerPool_t::~PeerPool_t()
{
	if (peer_mutex)
	{
		delete peer_mutex;
		peer_mutex = nullptr;
	}

	if (peer_threadpool_mutex)
	{
		delete peer_threadpool_mutex;
		peer_threadpool_mutex = nullptr;
	}
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
	return this->fncSleep;
}

void Net::PeerPool::PeerPool_t::set_max_peers(size_t max_peers)
{
	this->max_peers = max_peers;
}

bool Net::PeerPool::PeerPool_t::check_more_threads_needed()
{
	const std::lock_guard<std::mutex> lock(*peer_threadpool_mutex);
	for (const auto& pool : peer_threadpool)
	{
		if (pool->num_peers != max_peers)
			return false;
	}

	return true;
}

void Net::PeerPool::PeerPool_t::threapool_push(peer_threadpool_t* pool)
{
	const std::lock_guard<std::mutex> lock(*peer_threadpool_mutex);
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

	while (true)
	{
		bool take_rest = true;

		for (auto& peer : pool->vPeers)
		{
			if (peer == nullptr)
			{
				auto waiting_peer = pClass->queue_pop();
				if (waiting_peer)
				{
					peer = waiting_peer;
					pool->num_peers++;
				}
				else
				{
					continue;
				}
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

				delete peer;
				peer = nullptr;
				pool->num_peers--;
				break;
			}

			case Net::PeerPool::WorkStatus_t::CONTINUE:
			{
				const std::lock_guard<std::mutex> lock(*pClass->get_peer_threadpool_mutex());
				const auto p = pClass->threadpool_get_free_slot_in_target_pool(pool);
				if (p)
				{
					// move peer pointer
					for (auto& target_peer : p->vPeers)
					{
						if (!target_peer)
						{
							target_peer = peer;
							p->num_peers++;

							peer = nullptr;
							pool->num_peers--;
							break;
						}
					}
				}
				break;
			}

			// do not take a rest if we want to forward on processing worker function rapidly
			case Net::PeerPool::WorkStatus_t::FORWARD:
			{
				take_rest = false;
				break;
			}

			default:
				break;
			}
		}

		// close this thread
		if (pool->num_peers == 0)
		{
			// erase from vector
			const std::lock_guard<std::mutex> lock(*pClass->get_peer_threadpool_mutex());
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

			delete pool;
			pool = nullptr;
			return NULL;
		}

		if (take_rest)
		{
			auto fncSleepPointer = pClass->get_sleep_function();
			if (fncSleepPointer)
			{
				auto fncSleep = reinterpret_cast<void (*)(DWORD time)>(fncSleepPointer);
				(*fncSleep)(pClass->get_sleep_time());
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(pClass->get_sleep_time()));
			}
		}
	}

	LOG("THREAD ENDED");
	delete data;
	return NULL;
}

void Net::PeerPool::PeerPool_t::threadpool_add()
{
	peer_threadpool_t* pool = new peer_threadpool_t();

	// add nullptr's to dynamic vector to reserve the memory
	for (size_t i = 0; i < this->max_peers; i++)
		pool->vPeers.push_back(nullptr);

	pool->num_peers = 0;

	// dispatch the thread
	auto data = new threadpool_manager_data_t();
	data->pClass = this;
	data->pool = pool;
	Net::Thread::Create(threadpool_manager, (LPVOID)data);

	// push to vector
	threapool_push(pool);
}

Net::PeerPool::peer_threadpool_t* Net::PeerPool::PeerPool_t::threadpool_get_free_slot_in_target_pool(peer_threadpool_t* current_pool)
{
	if (!peer_queue.empty()) return nullptr;

	for (const auto& pool : peer_threadpool)
	{
		if (pool == current_pool) continue;
		for (const auto& peer : pool->vPeers)
			if (!peer) return pool;
	}

	return nullptr;
}

Net::PeerPool::peerInfo_t* Net::PeerPool::PeerPool_t::queue_pop()
{
	if (peer_queue.empty()) return nullptr;

	const std::lock_guard<std::mutex> lock(*peer_mutex);
	auto peer = peer_queue.back();
	peer_queue.pop_back();
	return peer;
}

std::vector<Net::PeerPool::peer_threadpool_t*>& Net::PeerPool::PeerPool_t::get_peer_threadpool()
{
	return this->peer_threadpool;
}

std::mutex* Net::PeerPool::PeerPool_t::get_peer_threadpool_mutex()
{
	return this->peer_threadpool_mutex;
}

void Net::PeerPool::PeerPool_t::add(peerInfo_t info)
{
	const std::lock_guard<std::mutex> lock(*peer_mutex);
	peer_queue.emplace_back(new peerInfo_t(info));

	if (check_more_threads_needed())
		this->threadpool_add();
}

void Net::PeerPool::PeerPool_t::add(peerInfo_t* info)
{
	const std::lock_guard<std::mutex> lock(*peer_mutex);
	peer_queue.emplace_back(info);

	if (check_more_threads_needed())
		this->threadpool_add();
}

size_t Net::PeerPool::PeerPool_t::count_peers_all()
{
	size_t peers = 0;

	const std::lock_guard<std::mutex> lock(*peer_threadpool_mutex);
	for (const auto& pool : peer_threadpool)
		for (const auto& peer : pool->vPeers)
			if (peer) peers++;

	return peers;
}

size_t Net::PeerPool::PeerPool_t::count_peers(peer_threadpool_t* pool)
{
	size_t peers = 0;

	for (const auto& peer : pool->vPeers)
		if (peer) peers++;

	return peers;
}

size_t Net::PeerPool::PeerPool_t::count_pools()
{
	return peer_threadpool.size();
}