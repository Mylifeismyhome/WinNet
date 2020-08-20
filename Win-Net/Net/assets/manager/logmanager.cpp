#include "logmanager.h"
#include "assets/assets.h"

void SetFname(const char* name)
{
	fname = name;
}

const char* GetFname()
{
	return fname;
}

void SetPath(const char* p)
{
	path = p;
}

char* GetPath()
{
	return (char*)path;
}

void SetAreaInUse(const bool status)
{
	AreaInUse = status;
}

bool IsAreaInUse()
{
	return AreaInUse;
}

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
Log::Log()
{
	if (strcmp(GetPath(), "") != 0)
	{
		NET_CREATEDIR(GetPath());
		const auto len = strlen(GetFname()) + strlen(GetPath()) + 6;
		auto fname = ALLOC<char>(len + 1);
		sprintf_s(fname, len, CSTRING("%s/%s.log"), GetPath(), GetFname());
		file = new NET_FILEMANAGER(fname, NET_FILE_APPAND | NET_FILE_READWRITE);
		FREE(fname);
	}
	else
	{
		const auto len = strlen(GetFname()) + 5;
		auto fname = ALLOC<char>(len + 1);
		sprintf_s(fname, len, CSTRING("%s.log"), GetFname());
		file = new NET_FILEMANAGER(fname, NET_FILE_APPAND | NET_FILE_READWRITE);
		FREE(fname);
	}
}

Log::~Log()
{
	if (file)
	{
		delete file;
		file = nullptr;
	}
}

void Log::doLog(LogManagerStates state, const char* func, const char* msg, ...) const
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vsnprintf(nullptr, 0, msg, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	if (strcmp(func, CSTRING("")) == 0)
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + 9;
		auto displayBuff = ALLOC<char>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		sprintf_s(displayBuff, DisplaySize, CSTRING("[%s] %s\n"), time, str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!Console::GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState((Console::LogStates)state));
			printf(CSTRING("%s"), displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		Console::LogInformations logInfo;
		logInfo.buffer = ALLOC<char>((DisplaySize - 2) + 1);
		memcpy(logInfo.buffer, displayBuff, DisplaySize - 2);
		logInfo.buffer[DisplaySize - 2] = '\0';
		logInfo.state = (Console::LogStates)state;
		InsertLog(logInfo);

		if (!WriteToFile(displayBuff))
			printf(CSTRING("%s"), CSTRING("Failure on writting std output to file!"));

		FREE(displayBuff);
	}
	else
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + strlen(func) + 11;
		auto displayBuff = ALLOC<char>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		sprintf_s(displayBuff, DisplaySize, CSTRING("[%s][%s] %s\n"), time, func, str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!Console::GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState((Console::LogStates)state));
			printf(CSTRING("%s"), displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		Console::LogInformations logInfo;
		logInfo.buffer = ALLOC<char>((DisplaySize - 2) + 1);
		memcpy(logInfo.buffer, displayBuff, DisplaySize - 2);
		logInfo.buffer[DisplaySize - 2] = '\0';
		logInfo.state = (Console::LogStates) state;
		InsertLog(logInfo);
		
		if (!WriteToFile(displayBuff))
			printf(CSTRING("%s"), CSTRING("Failure on writting std output to file!"));

		FREE(displayBuff);
	}
}

void Log::doLog(LogManagerStates state, const char* func, const wchar_t* msg, ...) const
{
	va_list vaArgs;
	va_start(vaArgs, msg);
	const size_t size = std::vswprintf(nullptr, 0, msg, vaArgs);
	std::vector<wchar_t> str(size + 1);
	std::vswprintf(str.data(), str.size(), msg, vaArgs);
	va_end(vaArgs);

	if (strcmp(func, CSTRING("")) == 0)
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + 9;
		auto displayBuff = ALLOC<wchar_t>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		swprintf_s(displayBuff, DisplaySize, CWSTRING(L"[%s] %s\n"), reinterpret_cast<const wchar_t*>(time), str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!Console::GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState((Console::LogStates)state));
			wprintf(CWSTRING(L"%s"), displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		Console::LogInformations logInfo;
		logInfo.wbuffer = ALLOC<wchar_t>((DisplaySize - 2) + 1);
		memcpy(logInfo.wbuffer, displayBuff, DisplaySize - 2);
		logInfo.wbuffer[DisplaySize - 2] = '\0';
		logInfo.state = (Console::LogStates)state;
		InsertLog(logInfo);
		
		if (!WriteToFile(displayBuff))
			printf(CSTRING("%s"), CSTRING("Failure on writting std output to file!"));

		FREE(displayBuff);
	}
	else
	{
		if (str.empty())
			return;

		const auto DisplaySize = str.size() + strlen(func) + 11;
		auto displayBuff = ALLOC<wchar_t>(DisplaySize + 1);

		char* time = nullptr;
		CURRENTTIME(time);
		swprintf_s(displayBuff, DisplaySize, CWSTRING(L"[%s][%s] %s\n"), reinterpret_cast<const wchar_t*>(time), reinterpret_cast<const wchar_t*>(func), str.data());
		FREE(time);
		displayBuff[DisplaySize] = '\0';

		if (!Console::GetPrintFState())
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorFromState((Console::LogStates)state));
			wprintf(CWSTRING(L"%s"), displayBuff);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE);
		}

		// store log
		Console::LogInformations logInfo;
		logInfo.wbuffer = ALLOC<wchar_t>((DisplaySize - 2) + 1);
		memcpy(logInfo.wbuffer, displayBuff, DisplaySize - 2);
		logInfo.wbuffer[DisplaySize - 2] = '\0';
		logInfo.state = (Console::LogStates)state;
		InsertLog(logInfo);

		if(!WriteToFile(displayBuff))
			printf(CSTRING("%s"), CSTRING("Failure on writting std output to file!"));

		FREE(displayBuff);
	}
}

bool Log::WriteToFile(const char* msg) const
{
	return file->write(msg);
}

bool Log::WriteToFile(const wchar_t* msg) const
{
	return file->write(msg);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
