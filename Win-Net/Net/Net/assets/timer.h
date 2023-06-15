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

#pragma once
#include <Net/Net/Net.h>
#include <Net/assets/thread.h>

#define NET_CONTINUE_TIMER return true
#define NET_STOP_TIMER return false
typedef bool NET_TimerRet;
#define NET_TIMER(fnc) NET_TimerRet fnc(void* param)
#define NET_HANDLE_TIMER Net::Timer::Timer_t*
typedef void(*OnNetTimerThreadFinished_t)(void* param);

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
			bool bdelete;
			OnNetTimerThreadFinished_t callback_onFinished;
			NET_THREAD_HANDLE hThread;
		};

		NET_HANDLE_TIMER Create(NET_TimerRet(*)(void*), double, void* = nullptr, bool = false);
		void Clear(NET_HANDLE_TIMER);
		void SetTime(NET_HANDLE_TIMER, double);
		NET_THREAD_DWORD WaitObject(NET_HANDLE_TIMER);
	}
}
