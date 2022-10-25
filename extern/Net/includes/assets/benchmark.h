#pragma once
#include <Net/Net.h>
#include <assets/timer.h>
#include <assets/assets.h>

enum class BenchmarkStates
{
	Log = 0,
	Write,
	LogAndWrite
};
#define BENCHMARK_STATE BenchmarkStates
#define BEGIN_BENCHMARK(fname, name, state) Net::Benchmark benchmark(fname, name, __FUNCSIG__, state, true);
#define WAIT_BEGIN_BENCHMARK(fname, name, state, allow) Net::Benchmark benchmark(fname, name, __FUNCSIG__, state, allow);
#define END_BENCHMARK benchmark.Stop();

BEGIN_NAMESPACE(Net)
BEGIN_CLASS(Benchmark)
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
CLASS_PUBLIC

CLASS_CONSTRUCTUR(Benchmark, const char*, const char*, const char*, BenchmarkStates, bool)
CLASS_DESTRUCTUR(Benchmark)
char* GetBenchmarkName() const;
char* GetBenchmarkFName() const;
void Start();
void Restart();
void Stop();
void DisplaySummary();
void writeResult();
END_CLASS
END_NAMESPACE