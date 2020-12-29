#include <Net/assets/timer.h>
#include <Net/Import/Kernel32.h>
#include <Net/assets/thread.h>

NET_THREAD(NetTimerThread)
{
	if (!parameter)
		return NULL;

	auto timer = (Net::Timer::Timer_t*)parameter;
	if (!timer->func)
	{
		timer->finished = true;
		NET_UNUSED_PARAM(timer->param);
		timer = nullptr;
		return NULL;
	}

	timer->last = clock();

	while (TRUE)
	{
		if (timer->clear)
		{
			timer->finished = true;
			NET_UNUSED_PARAM(timer->param);
			timer = nullptr;
			return NULL;
		}
		
		if ((clock() - timer->last) > timer->timer)
		{
			if (!(*timer->func)(timer->param))
			{
				timer->finished = true;
				NET_UNUSED_PARAM(timer->param);
				timer = nullptr;
				return NULL;
			}

			timer->last += timer->timer;
		}

		Net::Kernel32::Sleep(1);
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
	Thread::Create(NetTimerThread, timer_t);
	return timer_t;
}

void Net::Timer::Clear(NET_HANDLE_TIMER handle)
{
	if (!handle)
		return;

	handle->clear = true;
	handle = nullptr;
}

void Net::Timer::WaitSingleObjectStopped(NET_HANDLE_TIMER handle)
{
	if (!handle)
		return;

	handle->clear = true;
	while (!handle->finished) {};
	handle = nullptr;
}

void Net::Timer::SetTime(NET_HANDLE_TIMER handle, const double timer)
{
	if (!handle)
		return;

	handle->timer = timer;
}