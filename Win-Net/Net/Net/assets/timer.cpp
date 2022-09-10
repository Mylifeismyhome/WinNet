#include <Net/assets/timer.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/assets/thread.h>

NET_THREAD(NetTimerThread)
{
	if (!parameter)
		return NULL;

	auto timer = (Net::Timer::Timer_t*)parameter;
	if (!timer->func)
	{
		NET_UNUSED_PARAM(timer->param);

		if (timer->async)
		{
			NET_UNUSED(timer);
			return NULL;
		}

		timer->finished = true;
		return NULL;
	}

	timer->last = clock();

	while (true)
	{
		if (timer->clear)
		{
			NET_UNUSED_PARAM(timer->param);

			if (timer->async)
			{
				NET_UNUSED(timer);
				return NULL;
			}

			timer->finished = true;
			return NULL;
		}

		if ((clock() - timer->last) > timer->timer)
		{
			if (!(*timer->func)(timer->param))
			{
				NET_UNUSED_PARAM(timer->param);

				if (timer->async)
				{
					NET_UNUSED(timer);
					return NULL;
				}

				timer->finished = true;
				return NULL;
			}

			timer->last += timer->timer;
		}

#ifdef BUILD_LINUX
		usleep(1 * 1000);
#else
		Kernel32::Sleep(1);
#endif
	}
}

NET_HANDLE_TIMER Net::Timer::Create(NET_TimerRet(*func)(void*), const double timer, void* param, const bool bdelete)
{
	const auto timer_t = new Timer_t();
	timer_t->param = param;
	timer_t->func = func;
	timer_t->timer = timer;
	timer_t->clear = false;
	timer_t->finished = false;
	timer_t->bdelete = bdelete;
	timer_t->async = false;
	Thread::Create(NetTimerThread, timer_t);
	return timer_t;
}

void Net::Timer::Clear(NET_HANDLE_TIMER handle)
{
	if (!handle) return;
	handle->async = true;
	handle->clear = true;
}

void Net::Timer::WaitSingleObjectStopped(NET_HANDLE_TIMER handle)
{
	if (!handle) return;
	handle->async = false;
	handle->clear = true;
	WaitTimerFinished(handle);
	NET_UNUSED(handle);
}

void Net::Timer::SetTime(NET_HANDLE_TIMER handle, const double timer)
{
	if (!handle) return;
	handle->timer = timer;
}

void Net::Timer::WaitTimerFinished(NET_HANDLE_TIMER handle)
{
	if(!handle) return;
	while (!handle->finished) {};
}
