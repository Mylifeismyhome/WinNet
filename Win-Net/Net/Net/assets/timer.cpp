#include "timer.h"

NET_NAMESPACE_BEGIN(Net)
Timer::Timer()
{
	duration = -1;
	startTime = -1;
	endTime = -1;
	timepassed = 0;
	Active = false;
}

Timer::~Timer()
{
	Stop();
}

void Timer::Start()
{
	SetActive(true);
	start = std::chrono::high_resolution_clock::now();
}

void Timer::Restart()
{
	SetActive(true);
	start = std::chrono::high_resolution_clock::now();
}

void Timer::Stop()
{
	end = std::chrono::high_resolution_clock::now();

	const auto start_elapse = std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch().count();
	const auto end_elapse = std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count();
	startTime = static_cast<long long>(start_elapse * 0.001);
	endTime = static_cast<long long>(end_elapse * 0.001);

	const auto elapse = end_elapse - start_elapse;
	duration = elapse * 0.001;
	SetActive(false);
}

double Timer::GetElapse() const
{
	return duration;
}

void Timer::SetActive(const bool active)
{
	Active = active;
}

bool Timer::IsActive() const
{
	return Active;
}

long long Timer::GetStartTime() const
{
	return startTime;
}

long long Timer::GetEndTime() const
{
	return endTime;
}

double Timer::GetTimePassed() const
{
	const auto end = std::chrono::high_resolution_clock::now();

	const auto start_elapse = std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch().count();
	const auto end_elapse = std::chrono::time_point_cast<std::chrono::microseconds>(end).time_since_epoch().count();

	const auto elapse = end_elapse - start_elapse;
	const auto duration = elapse * 0.001;
	
	return duration;
}
NET_NAMESPACE_END
