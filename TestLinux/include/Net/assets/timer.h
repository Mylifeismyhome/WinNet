#pragma once
#include <Net/Net/Net.h>

#define NET_CONTINUE_TIMER return true
#define NET_STOP_TIMER return false
typedef bool NET_TimerRet;
#define NET_TIMER(fnc) NET_TimerRet fnc(void* param)
#define NET_HANDLE_TIMER Net::Timer::Timer_t*
#define NET_UNUSED_PARAM(param) if(timer->bdelete) delete param; param = nullptr;
#define NET_UNUSED(param) if(param->bdelete) delete param; param = nullptr;

namespace Net
{
	namespace Timer
	{
		struct Timer_t
		{
			NET_TimerRet(*func)(void*);
			void* param;
			double timer;
			double last;
			bool clear;
			bool finished;
			bool bdelete;
			bool async;
		};

		NET_HANDLE_TIMER Create(NET_TimerRet(*)(void*), double, void* = nullptr, bool = false);
		void Clear(NET_HANDLE_TIMER);
		void WaitSingleObjectStopped(NET_HANDLE_TIMER);
		void SetTime(NET_HANDLE_TIMER, double);
		void WaitTimerFinished(NET_HANDLE_TIMER);
	}
}
