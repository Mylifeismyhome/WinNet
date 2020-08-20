#pragma once
#include <Net/Net.h>

NET_DSA_BEGIN

#define NET_TIMER Net::Timer

NET_NAMESPACE_BEGIN(Net)
NET_CLASS_BEGIN(Timer)
std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> end;
double duration;
bool Active;

long long startTime;
long long endTime;
long long timepassed;
NET_CLASS_PUBLIC

NET_CLASS_CONSTRUCTUR(Timer)
NET_CLASS_DESTRUCTUR(Timer)

void Start();
void Restart();
void Stop();
double GetElapse() const;
void SetActive(bool);
bool IsActive() const;
long long GetStartTime() const;
long long GetEndTime() const;
double GetTimePassed() const;
NET_CLASS_END
NET_NAMESPACE_END

NET_DSA_END