#pragma once
#define BENCHMARK_STATE BenchmarkStates
#define BEGIN_BENCHMARK(fname, name, state) Net::Benchmark benchmark(fname, name, __FUNCSIG__, state, true);
#define WAIT_BEGIN_BENCHMARK(fname, name, state, allow) Net::Benchmark benchmark(fname, name, __FUNCSIG__, state, allow);
#define END_BENCHMARK benchmark.Stop();

#include <Net/Net/Net.h>
#include <Net/assets/timer.h>
#include <Net/assets/assets.h>
#include <Net/Net/NetString.h>

NET_NAMESPACE_BEGIN(Net)
NET_DSA_BEGIN
enum class BenchmarkStates
{
	Log = 0,
	Write,
	LogAndWrite
};

NET_CLASS_BEGIN(Benchmark)
char* benchmark_name;
char* func_name;
char* benchmark_fname;
NET_TIMER* timer;

// IRL TimeTable
TIMETABLE startClock;
WORD startMillis;
TIMETABLE endClock;
WORD endMillis;

BenchmarkStates bState;

void GetCPUBrand(char*&) const;
void GetCPUSpeed(DWORD&) const;
NET_CLASS_PUBLIC

NET_CLASS_CONSTRUCTUR(Benchmark, const char*, const char*, const char*, BenchmarkStates, bool)
NET_CLASS_DESTRUCTUR(Benchmark)
char* GetBenchmarkName() const;
char* GetBenchmarkFName() const;
void Start();
void Restart();
void Stop();
void DisplaySummary();
void writeResult();
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END