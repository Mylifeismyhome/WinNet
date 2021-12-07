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
			std::mutex* peer_mutex;

			std::vector<peer_threadpool_t*> peer_threadpool;
			std::mutex* peer_threadpool_mutex;

			bool check_more_threads_needed();
			void threapool_push(peer_threadpool_t* pool);
			void threadpool_add();

			DWORD ms_sleep_time;
			void (*fncSleep)(DWORD time);

			size_t max_peers;

		public:
			PeerPool_t();
			~PeerPool_t();

			peerInfo_t* queue_pop();

			std::vector<peer_threadpool_t*>& get_peer_threadpool();
			std::mutex* get_peer_threadpool_mutex();

			peer_threadpool_t* threadpool_get_free_slot_in_target_pool(peer_threadpool_t* from_pool);

			void set_sleep_time(DWORD ms_sleep_time);
			DWORD get_sleep_time();

			void set_sleep_function(void (*fncSleep)(DWORD time));
			void* get_sleep_function();

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