#include <Net/assets/timer.h>
#include <Net/Import/Kernel32.hpp>
#include <Net/assets/thread.h>

#ifndef BUILD_LINUX
static int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	Kernel32::GetSystemTime(&system_time);
	Kernel32::SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#else
#include <sys/time.h>
#endif

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

	struct timeval tv;
	gettimeofday(&tv, NULL);

	timer->last = static_cast<double>((((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000));

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

		gettimeofday(&tv, NULL);

		if (((((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000) - timer->last) > timer->timer)
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
	const auto timer_t = ALLOC<Timer_t>();
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
