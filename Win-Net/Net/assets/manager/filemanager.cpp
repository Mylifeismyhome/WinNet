#include "filemanager.h"

static const char* GetModeA(const uint8_t Mode)
{
	if (Mode == NET_FILE_WRITE)
		return "w";
	if (Mode == NET_FILE_READ)
		return "r";
	if (Mode == NET_FILE_APPAND)
		return "a";

	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return "a+";

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return "w+";

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return "r+";

	return "r";
}

static const wchar_t* GetModeW(const uint8_t Mode)
{
	if (Mode == NET_FILE_WRITE)
		return L"w";
	if (Mode == NET_FILE_READ)
		return L"r";
	if (Mode == NET_FILE_APPAND)
		return L"a";

	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return L"a+";

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return L"w+";

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return L"r+";

	return L"r";
}

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
FileManagerW::FileManagerW(const wchar_t* fname, const uint8_t Mode)
{
	file = nullptr;
	wcscpy_s(this->fname, fname);
	this->Mode = Mode;
}

FileManagerW::~FileManagerW()
{
	closeFile();
}

bool FileManagerW::openFile()
{
	_wfopen_s(&file, fname, GetModeW(Mode));
	if(!file)
	{
		_wfopen_s(&file, fname, GetModeW(NET_FILE_WRITE));
		close();
		_wfopen_s(&file, fname, GetModeW(Mode));
	}
	return file != nullptr;
}

void FileManagerW::closeFile()
{
	if (file)
	{
		fclose(file);
		file = nullptr;
	}
}

bool FileManagerW::CanOpenFile()
{
	_wfopen_s(&file, fname, GetModeW(Mode));
	const auto status = file != nullptr;
	closeFile();
	return status;
}

bool FileManagerW::getFileBuffer(BYTE*& out_data, size_t& out_size)
{
	fseek(file, 0, SEEK_END);
	const auto size = ftell(file);
	fseek(file, 0, NULL);

	auto buffer = ALLOC<char>(static_cast<size_t>(size) + 1);
	const auto read = fread(buffer, 1, size, file);
	if (read == NULL)
	{
		FREE(buffer);
		closeFile();
		return false;
	}

	buffer[size] = '\0';
	out_data = (BYTE*)buffer;
	out_size = static_cast<size_t>(size);
	closeFile();

	return true;
}

bool FileManagerW::file_exists()
{
	return CanOpenFile();
}

bool FileManagerW::read(BYTE*& out_data, size_t& out_size)
{
	if (file_exists())
	{
		openFile();
		return getFileBuffer(out_data, out_size);
	}

	return false;
}

bool FileManagerW::read(char*& out_data)
{
	if (file_exists())
	{
		openFile();

		BYTE* out_byte = nullptr;
		size_t out_size = NULL;
		const auto res = getFileBuffer(out_byte, out_size);
		if (res && out_size > 0)
		{
			out_data = ALLOC<char>(out_size + 1);
			memcpy(out_data, out_byte, out_size);
			out_data[out_size] = '\0';
		}

		FREE(out_byte);

		return res;
	}

	return false;
}

bool FileManagerW::write(BYTE* data, const size_t size)
{
	if (!openFile())
		return false;
	
	const auto written = fwrite(data, sizeof BYTE, size, file);
	close();
	return written != NULL;
}

bool FileManagerW::write(const char* str)
{
	if (!openFile())
		return false;
	
	const auto written = fwrite(str, 1, strlen(str), file);
	close();
	return written != NULL;
}

bool FileManagerW::write(const wchar_t* str)
{
	if (!openFile())
		return false;
	
	const auto written = fwrite(str, 2, wcslen(str), file);
	close();
	return written != NULL;
}

void FileManagerW::clear() const
{
	fflush(file);
}

void FileManagerW::close()
{
	closeFile();
}
FileManagerA::FileManagerA(const char* fname, const uint8_t Mode)
{
	file = nullptr;
	strcpy_s(this->fname, fname);
	this->Mode = Mode;
}

FileManagerA::~FileManagerA()
{
	closeFile();
}

bool FileManagerA::openFile()
{
	fopen_s(&file, fname, GetModeA(Mode));
	if (!file)
	{
		fopen_s(&file, fname, GetModeA(NET_FILE_WRITE));
		close();
		fopen_s(&file, fname, GetModeA(Mode));
	}
	return file != nullptr;
}

void FileManagerA::closeFile()
{
	if (file)
	{
		fclose(file);
		file = nullptr;
	}
}

bool FileManagerA::CanOpenFile()
{
	fopen_s(&file, fname, GetModeA(Mode));
	const auto status = file != nullptr;
	closeFile();
	return status;
}

bool FileManagerA::getFileBuffer(BYTE*& out_data, size_t& out_size)
{
	fseek(file, 0, SEEK_END);
	const auto size = ftell(file);
	fseek(file, 0, NULL);

	auto buffer = ALLOC<char>(static_cast<size_t>(size) + 1);
	const auto read = fread(buffer, 1, size, file);
	if (read == NULL)
	{
		FREE(buffer);
		closeFile();
		return false;
	}

	buffer[size] = '\0';
	out_data = (BYTE*)buffer;
	out_size = static_cast<size_t>(size);
	closeFile();

	return true;
}

bool FileManagerA::file_exists()
{
	return CanOpenFile();
}

bool FileManagerA::read(BYTE*& out_data, size_t& out_size)
{
	if (file_exists())
	{
		openFile();
		return getFileBuffer(out_data, out_size);
	}

	return false;
}

bool FileManagerA::read(char*& out_data)
{
	if (file_exists())
	{
		openFile();

		BYTE* out_byte = nullptr;
		size_t out_size = NULL;
		const auto res = getFileBuffer(out_byte, out_size);
		if (res && out_size > 0)
		{
			out_data = ALLOC<char>(out_size + 1);
			memcpy(out_data, out_byte, out_size);
			out_data[out_size] = '\0';
		}

		FREE(out_byte);

		return res;
	}

	return false;
}

bool FileManagerA::write(BYTE* data, const size_t size)
{
	if (!openFile())
		return false;

	const auto written = fwrite(data, sizeof BYTE, size, file);
	close();
	return written != NULL;
}

bool FileManagerA::write(const char* str)
{
	if (!openFile())
		return false;

	const auto written = fwrite(str, 1, strlen(str), file);
	close();
	return written != NULL;
}

bool FileManagerA::write(const wchar_t* str)
{
	if (!openFile())
		return false;

	const auto written = fwrite(str, 2, wcslen(str), file);
	close();
	return written != NULL;
}

void FileManagerA::clear() const
{
	fflush(file);
}

void FileManagerA::close()
{
	closeFile();
}
NET_NAMESPACE_END
NET_NAMESPACE_END
