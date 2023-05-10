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

#include <Net/Net/Net.h>
#include <Net/assets/thread.h>
#include <mutex>

NET_DSA_BEGIN
namespace Net
{
	namespace PeerPool
	{
		enum class WorkStatus_t
		{
			STOP = 0,
			CONTINUE,
			FORWARD
		};

		class peerInfo_t
		{
			void* peer;
			WorkStatus_t(*fncWork)(void* peer);
			void (*fncCallbackOnDelete)(void* peer);

		public:
			peerInfo_t();
			peerInfo_t(void* peer);
			peerInfo_t(void* peer, WorkStatus_t(*fncWork)(void* peer));
			peerInfo_t(void* peer, void (*fncCallbackOnDelete)(void* peer));
			peerInfo_t(void* peer, WorkStatus_t(*fncWork)(void* peer), void (*fncCallbackOnDelete)(void* peer));

			void SetPeer(void* peer);
			void* GetPeer();

			void SetWorker(WorkStatus_t(*fncWork)(void* peer));
			void* GetWorker();

			void SetCallbackOnDelete(void (*fncCallbackOnDelete)(void* peer));
			void* GetCallbackOnDelete();
		};

		struct peer_threadpool_t
		{
			Net::PeerPool::peerInfo_t** vPeers;
		};

		class PeerPool_t
		{
			std::vector<Net::PeerPool::peerInfo_t*> peer_queue;
			std::recursive_mutex peer_mutex;

			std::vector<peer_threadpool_t*> peer_threadpool;
			std::recursive_mutex peer_threadpool_mutex;

			bool check_more_threads_needed();
			void threapool_push(peer_threadpool_t* pool);
			void threadpool_add();

			size_t max_peers;

		public:
			PeerPool_t();

			peerInfo_t* queue_pop();

			std::vector<peer_threadpool_t*>& get_peer_threadpool();
			std::recursive_mutex* get_peer_threadpool_mutex();

			peer_threadpool_t* threadpool_get_free_slot_in_target_pool(peer_threadpool_t* from_pool);

			void set_max_peers(size_t max_peers);
			size_t get_max_peers();

			void add(peerInfo_t);
			void add(peerInfo_t*);

			size_t count_peers_all();
			size_t count_peers(peer_threadpool_t* pool);
			size_t count_pools();
		};
	}
}
NET_DSA_END