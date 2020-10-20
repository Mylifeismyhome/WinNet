#include "filemanager.h"

static const char* GetModeA(const uint8_t Mode)
{
	if (Mode == NET_FILE_WRITE)
		return "wb";
	if (Mode == NET_FILE_READ)
		return "rb";
	if (Mode == NET_FILE_APPAND)
		return "ab";

	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return "ab+";

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return "wb+";

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return "rb+";

	return "rb";
}

static const wchar_t* GetModeW(const uint8_t Mode)
{
	if (Mode == NET_FILE_WRITE)
		return L"wb";
	if (Mode == NET_FILE_READ)
		return L"rb";
	if (Mode == NET_FILE_APPAND)
		return L"ab";

	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return L"ab+";

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return L"wb+";

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return L"rb+";

	return L"rb";
}

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
FileManagerErrorRef::FileManagerErrorRef(ErrorCodes code)
{
	strerror_s(buffer, ERRORCODEDESC_LEN, (int)code);
}

FileManagerErrorRef::~FileManagerErrorRef()
{
	memset(buffer, NULL, ERRORCODEDESC_LEN);
}

char* FileManagerErrorRef::get()
{
	return buffer;
}

char* FileManagerErrorRef::data()
{
	return buffer;
}

char* FileManagerErrorRef::str()
{
	return buffer;
}

FileManagerW::FileManagerW(const wchar_t* fname, const uint8_t Mode)
{
	err = (errno_t)ErrorCodes::ERR_OK;
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
	err = _wfopen_s(&file, fname, GetModeW(Mode));
	if(!file)
	{
		err = _wfopen_s(&file, fname, GetModeW(NET_FILE_WRITE));
		close();
		err = _wfopen_s(&file, fname, GetModeW(Mode));
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
	err = _wfopen_s(&file, fname, GetModeW(NET_FILE_READ));
	const auto status = getLastError() != ErrorCodes::ERR_NOENT;
	closeFile();
	return status;
}

bool FileManagerW::getFileBuffer(BYTE*& out_data, size_t& out_size)
{
	fseek(file, 0, SEEK_END);
	const auto size = static_cast<size_t>(ftell(file));
	rewind(file);

	auto buffer = ALLOC<BYTE>(size + 1);
	const auto read = fread(buffer, 1, size, file);
	if (read != size)
	{
		FREE(buffer);
		closeFile();
		return false;
	}

	buffer[read] = '\0';
	out_data = buffer;
	out_size = read;
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
		out_data = res ? reinterpret_cast<char*>(out_byte) : nullptr;
		return res;
	}

	return false;
}

bool FileManagerW::write(BYTE* data, const size_t size)
{
	if (!openFile())
		return false;
	
	const auto written = fwrite(data, 1, size, file);
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

ErrorCodes FileManagerW::getLastError() const
{
	return (ErrorCodes)err;
}

FileManagerErrorRef FileManagerW::ErrorDescription(const ErrorCodes code) const
{
	return FileManagerErrorRef(code);
}

FileManagerA::FileManagerA(const char* fname, const uint8_t Mode)
{
	err = (errno_t)ErrorCodes::ERR_OK;
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
	err = fopen_s(&file, fname, GetModeA(Mode));
	if (!file)
	{
		err = fopen_s(&file, fname, GetModeA(NET_FILE_WRITE));
		close();
		err = fopen_s(&file, fname, GetModeA(Mode));
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
	err = fopen_s(&file, fname, GetModeA(NET_FILE_READ));
	const auto status = getLastError() != ErrorCodes::ERR_NOENT;
	closeFile();
	return status;
}

bool FileManagerA::getFileBuffer(BYTE*& out_data, size_t& out_size)
{
	fseek(file, 0, SEEK_END);
	const auto size = static_cast<size_t>(ftell(file));
	rewind(file);

	auto buffer = ALLOC<BYTE>(size + 1);
	const auto read = fread(buffer, 1, size, file);
	if (read != size)
	{
		FREE(buffer);
		closeFile();
		return false;
	}

	buffer[read] = '\0';
	out_data = buffer;
	out_size = read;
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
		out_data = res ? reinterpret_cast<char*>(out_byte) : nullptr;
		return res;
	}

	return false;
}

bool FileManagerA::write(BYTE* data, const size_t size)
{
	if (!openFile())
		return false;

	const auto written = fwrite(data, 1, size, file);
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

ErrorCodes FileManagerA::getLastError() const
{
	return (ErrorCodes)err;
}

FileManagerErrorRef FileManagerA::ErrorDescription(const ErrorCodes code) const
{
	return FileManagerErrorRef(code);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
