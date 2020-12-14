#pragma once
#include <Net/Net/Net.h>
#include <Net/assets/timer.h>
#include <ctime>

#include <Net/Import/Kernel32.h>
#include <Net/assets/thread.h>

#define CONTINUE_TIMER return true
#define STOP_TIMER return false
typedef BOOL TimerRet;
#define TIMER(fnc) TimerRet fnc(void* param)
#define HANDLE_TIMER Net::Timer::Timer_t*
#define UNUSED_PARAM(param) delete param

namespace Net
{
	namespace Timer
	{
		struct Timer_t
		{
			TimerRet(*func)(void*);
			void* param;
			double timer;
			double last;
			bool clear;
			bool finished;
		};

		HANDLE_TIMER Create(TimerRet(*)(void*), double, void* = nullptr);
		void Clear(HANDLE_TIMER);
		void WaitSingleObjectStopped(HANDLE_TIMER);
		void SetTime(HANDLE_TIMER, double);
	}
}