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

#include <Net/assets/timer.h>
#include <Net/Import/Kernel32.hpp>

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

static void OnNetTimerThreadFinished(void* param)
{
	if (param == nullptr)
	{
		return;
	}

	auto pTimer = reinterpret_cast<Net::Timer::Timer_t*>(param);
	Net::Thread::Close(pTimer->hThread);
	pTimer->hThread = 0;

	if (pTimer->param)
	{
		if (pTimer->bdelete)
		{
			delete pTimer->param;
			pTimer->param = nullptr;
		}
	}
}

NET_THREAD(NetTimerThread)
{
	if (parameter == nullptr)
	{
		return NULL;
	}

	auto timer = (Net::Timer::Timer_t*)parameter;
	if (timer->func == nullptr)
	{
		if (timer->callback_onFinished)
		{
			timer->callback_onFinished(timer);
		}

		return NULL;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);

	timer->last = static_cast<double>((((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000));

	while (1)
	{
		if (timer->clear)
		{
			if (timer->callback_onFinished)
			{
				timer->callback_onFinished(timer);
			}

			return NULL;
		}

		gettimeofday(&tv, NULL);

		if (((((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000) - timer->last) > timer->timer)
		{
			if ((*timer->func)(timer->param) == 0)
			{
				if (timer->callback_onFinished)
				{
					timer->callback_onFinished(timer);
				}

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
	const auto pTimer = ALLOC<Timer_t>();
	pTimer->param = param;
	pTimer->func = func;
	pTimer->timer = timer;
	pTimer->clear = false;
	pTimer->bdelete = bdelete;
	pTimer->callback_onFinished = &OnNetTimerThreadFinished;
	pTimer->hThread = Net::Thread::Create(NetTimerThread, pTimer);
	return pTimer;
}

void Net::Timer::Clear(NET_HANDLE_TIMER handle)
{
	if (handle == nullptr)
	{
		return;
	}

	handle->clear = true;
	Net::Timer::WaitObject(handle);
	FREE<Timer_t>(handle);
}

NET_THREAD_DWORD Net::Timer::WaitObject(NET_HANDLE_TIMER handle)
{
	if (handle == nullptr)
	{
		return 0;
	}

	if (handle->hThread == 0)
	{
		return 0;
	}

	handle->clear = true;
	return Net::Thread::WaitObject(handle->hThread);
}

void Net::Timer::SetTime(NET_HANDLE_TIMER handle, const double timer)
{
	if (handle == nullptr)
	{
		return;
	}

	handle->timer = timer;
}