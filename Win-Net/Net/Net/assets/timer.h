#pragma once
#include <Net/Net/Net.h>
#include <Net/assets/timer.h>
#include <ctime>

#include <Net/Import/Kernel32.h>
#include <Net/assets/thread.h>

#define NET_CONTINUE_TIMER return true
#define NET_STOP_TIMER return false
typedef BOOL NET_TimerRet;
#define NET_TIMER(fnc) NET_TimerRet fnc(void* param)
#define NET_HANDLE_TIMER Net::Timer::Timer_t*
#define NET_UNUSED_PARAM(param) delete param

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
		};

		NET_HANDLE_TIMER Create(NET_TimerRet(*)(void*), double, void* = nullptr);
		void Clear(NET_HANDLE_TIMER);
		void WaitSingleObjectStopped(NET_HANDLE_TIMER);
		void SetTime(NET_HANDLE_TIMER, double);
	}
}