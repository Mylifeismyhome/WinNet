#include <Net/assets/timer.h>

THREAD(NetTimerThread)
{
	if (!parameter)
		return NULL;

	auto timer = (Net::Timer::Timer_t*)parameter;
	if (!timer->func)
	{
		timer->finished = true;
		UNUSED_PARAM(timer->param);
		delete timer;
		timer = nullptr;
		return NULL;
	}

	timer->last = clock();

	while (TRUE)
	{
		if (timer->clear)
		{
			timer->finished = true;
			UNUSED_PARAM(timer->param);
			delete timer;
			timer = nullptr;
			return NULL;
		}
		
		if ((clock() - timer->last) > timer->timer)
		{
			if (!(*timer->func)(timer->param))
			{
				timer->finished = true;
				UNUSED_PARAM(timer->param);
				delete timer;
				timer = nullptr;
				return NULL;
			}

			timer->last += timer->timer;
		}

		Net::Kernel32::Sleep(1);
	}
}

NET_HANDLE_TIMER Net::Timer::Create(TimerRet(*func)(void*), const double timer, void* param)
{
	const auto timer_t = new Timer_t();
	timer_t->param = param;
	timer_t->func = func;
	timer_t->timer = timer;
	timer_t->clear = false;
	timer_t->finished = false;
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
	handle = nullptr;
}