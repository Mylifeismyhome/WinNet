#pragma once
#include <Net/Net.h>

#define NET_TIMER Net::Timer

BEGIN_NAMESPACE(Net)
BEGIN_CLASS(Timer)
std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> end;
double duration;
bool Active;

long long startTime;
long long endTime;
long long timepassed;
CLASS_PUBLIC

CLASS_CONSTRUCTUR(Timer)
CLASS_DESTRUCTUR(Timer)

void Start();
void Restart();
void Stop();
double GetElapse() const;
void SetActive(bool);
bool IsActive() const;
long long GetStartTime() const;
long long GetEndTime() const;
double GetTimePassed() const;
END_CLASS
END_NAMESPACE