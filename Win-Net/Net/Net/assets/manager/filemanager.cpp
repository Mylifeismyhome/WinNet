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

#include "filemanager.h"

NET_IGNORE_CONVERSION_NULL
#ifdef BUILD_LINUX
static FILE* __convertWfname2Afname__fopen(const wchar_t* fname, const wchar_t* Mode)
{
	char* fnameA = nullptr;
	wcstombs(fnameA, fname, wcslen(fname));

	char* ModeA = nullptr;
	wcstombs(ModeA, Mode, wcslen(Mode));

	FILE* f = fopen(fnameA, ModeA);

	FREE<char>(fnameA);
	FREE<char>(ModeA);

	return f;
}

#define wOpenFile(fname, mode) __convertWfname2Afname__fopen(fname, mode)
#define OpenFile(fname, mode) fopen(fname, mode)
#else
#define wOpenFile(fname, mode) _wfopen(fname, mode)
#define OpenFile(fname, mode) fopen(fname, mode)
#endif

static const char* GetModeA(const uint8_t Mode)
{
	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return ("ab+");

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return ("wb+");

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return ("rb+");

	if (Mode == NET_FILE_WRITE || Mode == NET_FILE_DISCARD)
		return ("wb");
	if (Mode == NET_FILE_READ)
		return ("rb");
	if (Mode == NET_FILE_APPAND)
		return ("ab");

	return ("rb");
}

static const wchar_t* GetModeW(const uint8_t Mode)
{
	if (Mode == (NET_FILE_READ | NET_FILE_APPAND)
		|| Mode == (NET_FILE_WRITE | NET_FILE_APPAND)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND))
		return (L"ab+");

	if (Mode == (NET_FILE_READ | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_WRITE | NET_FILE_DISCARD)
		|| Mode == (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_DISCARD))
		return (L"wb+");

	if (Mode == (NET_FILE_READ | NET_FILE_WRITE))
		return (L"rb+");

	if (Mode == NET_FILE_WRITE || Mode == NET_FILE_DISCARD)
		return (L"wb");
	if (Mode == NET_FILE_READ)
		return (L"rb");
	if (Mode == NET_FILE_APPAND)
		return (L"ab");

	return (L"rb");
}

static Net::Manager::FileManagerErrorRef GetErrorDescription(const Net::Manager::ErrorCodes code)
{
	return Net::Manager::FileManagerErrorRef(code);
}

namespace Net
{
	namespace Manager
	{
		FileManagerErrorRef::FileManagerErrorRef(ErrorCodes code)
		{
#ifdef BUILD_LINUX
			strerror_r((int)code, buffer, ERRORCODEDESC_LEN);
#else
			strerror_s(buffer, ERRORCODEDESC_LEN, (int)code);
#endif
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
			wcscpy(this->fname, fname);
			this->Mode = Mode;
		}

		FileManagerW::~FileManagerW()
		{
			closeFile();
		}

		bool FileManagerW::openFile()
		{
			file = wOpenFile(fname, GetModeW(Mode));
			if (!file)
			{
				err = errno;
				file = wOpenFile(fname, GetModeW(NET_FILE_WRITE));
				close();
				err = errno;
				file = wOpenFile(fname, GetModeW(Mode));
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
			file = wOpenFile(fname, GetModeW(NET_FILE_READ));
			err = errno;
			const auto status = ((file != nullptr) ? true : false);
			closeFile();
			return status;
		}

		bool FileManagerW::getFileBuffer(BYTE*& out_data, size_t& out_size) const
		{
			fseek(file, 0, SEEK_END);
			const auto size = static_cast<size_t>(ftell(file));
			rewind(file);

			auto buffer = ALLOC<BYTE>(size + 1);
			const auto read = fread(buffer, 1, size, file);
			if (read != size)
			{
				FREE<byte>(buffer);
				return false;
			}

			buffer[read] = '\0';
			out_data = buffer;
			out_size = read;

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
				if (!openFile())
					return false;

				const auto ret = getFileBuffer(out_data, out_size);
				close();
				return ret;
			}

			return false;
		}

		bool FileManagerW::read(char*& out_data)
		{
			if (file_exists())
			{
				if (!openFile())
					return false;

				BYTE* out_byte = nullptr;
				size_t out_size = NULL;
				const auto res = getFileBuffer(out_byte, out_size);
				out_data = res ? reinterpret_cast<char*>(out_byte) : nullptr;
				close();
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

		bool FileManagerW::size(size_t& size)
		{
			if (file_exists())
			{
				if (!openFile())
					return false;

				fseek(file, 0, SEEK_END);
				const auto size = static_cast<size_t>(ftell(file));
				rewind(file);
				return true;
			}

			return false;
		}

		void FileManagerW::flush()
		{
			auto prev_mode = this->Mode;

			/*
			* open with mode discard
			*/
			this->closeFile();
			this->Mode = NET_FILE_DISCARD;
			if (!this->openFile())
			{
				return;
			}

			/*
			* open with stored mode
			*/
			this->closeFile();
			this->Mode = prev_mode;
			if (!this->openFile())
			{
				return;
			}
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
			return GetErrorDescription(code);
		}

		FileManagerA::FileManagerA(const char* fname, const uint8_t Mode)
		{
			err = (errno_t)ErrorCodes::ERR_OK;
			file = nullptr;
			strcpy(this->fname, fname);
			this->Mode = Mode;
		}

		FileManagerA::~FileManagerA()
		{
			closeFile();
		}

		bool FileManagerA::openFile()
		{
			file = OpenFile(fname, GetModeA(Mode));
			err = errno;
			if (!file)
			{
				file = OpenFile(fname, GetModeA(NET_FILE_WRITE));
				err = errno;
				close();
				file = OpenFile(fname, GetModeA(Mode));
				err = errno;
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
			file = OpenFile(fname, GetModeA(NET_FILE_READ));
			err = errno;
			const auto status = ((file != nullptr) ? true : false);
			closeFile();
			return status;
		}

		bool FileManagerA::getFileBuffer(BYTE*& out_data, size_t& out_size) const
		{
			fseek(file, 0, SEEK_END);
			const auto size = static_cast<size_t>(ftell(file));
			rewind(file);

			auto buffer = ALLOC<BYTE>(size + 1);
			const auto read = fread(buffer, 1, size, file);
			if (read != size)
			{
				FREE<byte>(buffer);
				return false;
			}

			buffer[read] = '\0';
			out_data = buffer;
			out_size = read;

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
				if (!openFile())
					return false;

				const auto ret = getFileBuffer(out_data, out_size);
				close();
				return ret;
			}

			return false;
		}

		bool FileManagerA::read(char*& out_data)
		{
			if (file_exists())
			{
				if (!openFile())
					return false;

				BYTE* out_byte = nullptr;
				size_t out_size = NULL;
				const auto res = getFileBuffer(out_byte, out_size);
				out_data = res ? reinterpret_cast<char*>(out_byte) : nullptr;
				close();
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

		bool FileManagerA::size(size_t& size)
		{
			if (file_exists())
			{
				if (!openFile())
					return false;

				fseek(file, 0, SEEK_END);
				const auto size = static_cast<size_t>(ftell(file));
				rewind(file);
				return true;
			}

			return false;
		}

		void FileManagerA::flush()
		{
			auto prev_mode = this->Mode;

			/*
			* open with mode discard
			*/
			this->closeFile();
			this->Mode = NET_FILE_DISCARD;
			if (!this->openFile())
			{
				return;
			}

			/*
			* open with stored mode
			*/
			this->closeFile();
			this->Mode = prev_mode;
			if (!this->openFile())
			{
				return;
			}
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
			return GetErrorDescription(code);
		}
	}
}
NET_POP
