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

#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

typedef int errno_t;

NET_DSA_BEGIN

#ifdef BUILD_LINUX
#define NET_MAX_PATH PATH_MAX
#else
#define NET_MAX_PATH MAX_PATH
#endif

#ifdef UNICODE
#define NET_FILEMANAGER Net::Manager::FileManagerW
#else
#define NET_FILEMANAGER Net::Manager::FileManagerA
#endif

#define NET_FILEMANAGERW Net::Manager::FileManagerW
#define NET_FILEMANAGERA Net::Manager::FileManagerA

#define NET_FILE_APPAND (1 << 0)
#define NET_FILE_READ (1 << 1)
#define NET_FILE_WRITE (1 << 2)
#define NET_FILE_DISCARD (1 << 3)
#define NET_FILE_READWRITE (NET_FILE_READ | NET_FILE_WRITE)
#define NET_FILE_READWRITE_APPEND (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND)

#define ERRORCODEDESC_LEN 256

namespace Net
{
	namespace Manager
	{
		enum class ErrorCodes
		{
			ERR_OK = 0,
			ERR_PERM,
			ERR_NOENT,
			ERR_SRCH,
			ERR_INTR,
			ERR_IO,
			ERR_NXIO,
			ERR_2BIG,
			ERR_NOEXEC,
			ER_RBADF,
			ERR_CHILD,
			ERR_AGAIN,
			ERR_NOMEM,
			ERR_ACCES,
			ERR_FAULT,
			ERR_NOTBLK,
			ERR_BUSY,
			ERR_EXIST,
			ERR_XDEV,
			ERR_NODEV,
			ERR_NOTDIR,
			ERR_ISDIR,
			ERR_INVAL,
			ERR_NFILE,
			ERR_MFILE,
			ERR_NOTTY,
			ERR_TXTBSY,
			ERR_FBIG,
			ERR_NOSPC,
			ERR_SPIPE,
			ERR_ROFS,
			ERR_MLINK,
			ERR_PIPE,
			ERR_DOM,
			ERR_RANGE
		};

		class FileManagerErrorRef
		{
			char buffer[ERRORCODEDESC_LEN];

		public:
			FileManagerErrorRef(ErrorCodes);
			~FileManagerErrorRef();

			char* get();
			char* data();
			char* str();
		};

		class FileManagerW
		{
			wchar_t fname[NET_MAX_PATH];
			FILE* file;
			uint8_t Mode;
			errno_t err;

			bool getFileBuffer(BYTE*&, size_t&) const;

		public:
			FileManagerW(const wchar_t*, uint8_t = NET_FILE_READWRITE_APPEND);
			~FileManagerW();

			bool openFile();
			void closeFile();
			bool CanOpenFile();
			bool file_exists();
			bool read(BYTE*&, size_t&);
			bool read(char*&);
			bool write(BYTE*, size_t);
			bool write(const char*);
			bool write(const wchar_t*);
			bool size(size_t&);
			void flush();
			void close();
			ErrorCodes getLastError() const;
			FileManagerErrorRef ErrorDescription(ErrorCodes) const;
		};

		class FileManagerA
		{
			char fname[NET_MAX_PATH];
			FILE* file;
			uint8_t Mode;
			errno_t err;

			bool getFileBuffer(BYTE*&, size_t&) const;
		public:
			FileManagerA(const char*, uint8_t = NET_FILE_READWRITE_APPEND);
			~FileManagerA();

			bool openFile();
			void closeFile();
			bool CanOpenFile();
			bool file_exists();
			bool read(BYTE*&, size_t&);
			bool read(char*&);
			bool write(BYTE*, size_t);
			bool write(const char*);
			bool write(const wchar_t*);
			bool size(size_t&);
			void flush();
			void close();
			ErrorCodes getLastError() const;
			FileManagerErrorRef ErrorDescription(ErrorCodes) const;
		};
	}
}
NET_DSA_END