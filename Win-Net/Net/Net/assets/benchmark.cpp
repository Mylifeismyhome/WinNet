#include "benchmark.h"

NET_NAMESPACE_BEGIN(Net)
Benchmark::Benchmark(const char* fname, const char* name, const char* funcName, const BenchmarkStates state, const bool allow)
{
	const auto size_fname = strlen(fname);
	benchmark_fname = ALLOC<char>(size_fname + 1);
	memcpy(benchmark_fname, fname, size_fname);
	benchmark_fname[size_fname] = '\0';
	
	const auto size_name = strlen(name);
	benchmark_name = ALLOC<char>(size_name + 1);
	memcpy(benchmark_name, fname, size_name);
	benchmark_name[size_name] = '\0';
	
	const auto size_funcName = strlen(funcName);
	func_name = ALLOC<char>(size_funcName + 1);
	memcpy(func_name, fname, size_funcName);
	func_name[size_funcName] = '\0';

	bState = state;

	startClock = tm();
	startMillis = NULL;
	endClock = tm();
	endMillis = NULL;

	timer = nullptr;

	if (allow)
		Start();
}

Benchmark::~Benchmark()
{
	if (timer && timer->IsActive())
		Stop();

	delete timer;
}

char* Benchmark::GetBenchmarkName() const
{
	return benchmark_name;
}

char* Benchmark::GetBenchmarkFName() const
{
	return benchmark_fname;
}

void Benchmark::Start()
{
	timer = ALLOC<NET_TIMER>();
	timer->Start();
	startClock = Console::TM_GetTime();

	SYSTEMTIME time;
	GetSystemTime(&time);
	startMillis = time.wMilliseconds;
}

void Benchmark::Restart()
{
	timer->Restart();
	startClock = Console::TM_GetTime();

	SYSTEMTIME time;
	GetSystemTime(&time);
	startMillis = time.wMilliseconds;
}

void Benchmark::Stop()
{
	// Only Stop and show result, if timer been active
	if (timer && timer->IsActive())
	{
		timer->Stop();
		endClock = Console::TM_GetTime();

		SYSTEMTIME time;
		GetSystemTime(&time);
		endMillis = time.wMilliseconds;

		DisplaySummary();
	}
}

void Benchmark::GetCPUBrand(char*& out) const
{
	// Get extended ids.
	int CPUInfo[4] = { -1 };
	__cpuid(CPUInfo, 0x80000000);
	const auto nExIds = CPUInfo[0];

	// Get the information associated with each extended ID.
	char CPUBrand[0x40] = { 0 };
	for (auto i = 0x80000000; i <= static_cast<unsigned int>(nExIds); ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string and cache information.
		if (i == 0x80000002)
		{
			memcpy(CPUBrand,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000003)
		{
			memcpy(CPUBrand + 16,
				CPUInfo,
				sizeof(CPUInfo));
		}
		else if (i == 0x80000004)
		{
			memcpy(CPUBrand + 32, CPUInfo, sizeof(CPUInfo));
		}
	}

	FREE(out);
	out = ALLOC<char>(0x41);
	memcpy(out, CPUBrand, 0x40);
}

void Benchmark::GetCPUSpeed(DWORD& out) const
{
	char* Buffer[_MAX_PATH];
	DWORD BufSize = _MAX_PATH;
	DWORD dwMHz = _MAX_PATH;
	HKEY hKey;

	// open the key where the proc speed is hidden:
	const auto lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		(LPCSTR)CSTRING("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
		0,
		KEY_READ,
		&hKey);
	
	if (lError != ERROR_SUCCESS)
	{
		// if the key is not found, tell the user why:
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			lError,
			0,
			(LPSTR)Buffer,
			_MAX_PATH,
			nullptr);

		out = 0;
		return;
	}

	// query the key:
	RegQueryValueEx(hKey, (LPCSTR)CSTRING("~MHz"), nullptr, nullptr, (LPBYTE)&dwMHz, &BufSize);
	out = dwMHz;
}

void Benchmark::DisplaySummary()
{
	if (bState == BENCHMARK_STATE::Log || bState == BENCHMARK_STATE::LogAndWrite)
	{
		const auto ms = timer->GetElapse();
		LOG_DEBUG(CSTRING("---------- BEGIN BENCHMARK SUMMARY ----------"));
		LOG_DEBUG(CSTRING("Benchmark Name: %s"), GetBenchmarkName());
		LOG_DEBUG(CSTRING("Code execution took: %.3f ms"), ms);

		char* CPUBrand = nullptr;
		GetCPUBrand(CPUBrand);
		LOG_DEBUG(CSTRING("CPU Brand: %s"), CPUBrand);
		FREE(CPUBrand);

		DWORD CPUSpeed = 0;
		GetCPUSpeed(CPUSpeed);
		LOG_DEBUG(CSTRING("CPU Speed: %d MHz"), CPUSpeed);
		LOG_DEBUG(CSTRING("---------- END BENCHMARK SUMMARY ----------"));
	}

	if (bState == BENCHMARK_STATE::Write || bState == BENCHMARK_STATE::LogAndWrite)
		writeResult();
}

void Benchmark::writeResult()
{
	const auto fsize = strlen(GetBenchmarkFName());
	const auto ssize = strlen(CSTRING(".json"));
	auto name = ALLOC<char>((fsize + ssize) + 1);
	memcpy(&name[0], GetBenchmarkFName(), fsize);
	memcpy(&name[fsize], CSTRING(".json"), ssize);
	name[(fsize + ssize)] = '\0';
	NET_FILEMANAGER fmanager(name, NET_FILE_APPAND | NET_FILE_READWRITE);

	char* old = nullptr;
	fmanager.read(old);
	fmanager.clear();

	std::string entry;

	if (old && strcmp(old, CSTRING("")) != 0)
	{
		const auto NewSize = (strlen(old) - 2);
		auto New = ALLOC<char>(NewSize + 1);
		memset(New, NULL, NewSize);

		const auto oldSize = strlen(old);
		for (auto i = 0, j = 0; i < oldSize; ++i)
		{
			if (old[i] != '[' && old[i] != ']')
			{
				New[j] = old[i];
				j++;
			}
		}
		New[NewSize] = '\0';

		FREE(old);
		old = nullptr;
		fmanager.write(CSTRING("["));
		fmanager.write(New);
		FREE(New);

		entry = CSTRING(",{\")RowNameCSTRING(\":\")");
	}
	else
	{
		fmanager.write(CSTRING("["));
		entry = CSTRING("{\")RowNameCSTRING(\":\")");
	}

	FREE(old);

	entry += GetBenchmarkName();
	entry += CSTRING("CellName\n");
	entry += func_name;
	entry += CSTRING("timeStart:\n");
	entry += std::to_string(startClock.tm_hour);
	entry += CSTRING("min\n");
	entry += std::to_string(startClock.tm_min);
	entry += CSTRING("sec\n");
	entry += std::to_string(startClock.tm_sec);
	entry += CSTRING("ms\n");
	entry += std::to_string(startMillis);
	entry += CSTRING("timeEnd:\n");
	entry += std::to_string(endClock.tm_hour);
	entry += CSTRING("min:\n");
	entry += std::to_string(endClock.tm_min);
	entry += CSTRING("sec:\n");
	entry += std::to_string(endClock.tm_sec);
	entry += CSTRING("ms:\n");
	entry += std::to_string(endMillis);
	fmanager.write(entry.c_str());
	fmanager.write(CSTRING("]"));
	entry.clear();
	FREE(name);
}
NET_NAMESPACE_END
