#pragma once
#ifdef BUILD_LINUX
#else
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

NET_DSA_BEGIN

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

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Manager)
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

NET_CLASS_BEGIN(FileManagerErrorRef)
char buffer[ERRORCODEDESC_LEN];

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(FileManagerErrorRef, ErrorCodes);
NET_CLASS_DESTRUCTUR(FileManagerErrorRef);

char* get();
char* data();
char* str();
NET_CLASS_END

NET_CLASS_BEGIN(FileManagerW)
wchar_t fname[MAX_PATH];
FILE* file;
uint8_t Mode;
errno_t err;

bool getFileBuffer(BYTE*&, size_t&) const;
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(FileManagerW, const wchar_t*, uint8_t = NET_FILE_READWRITE_APPEND)
NET_CLASS_DESTRUCTUR(FileManagerW)
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
void clear()const;
void close();
ErrorCodes getLastError() const;
FileManagerErrorRef ErrorDescription(ErrorCodes) const;
NET_CLASS_END
NET_CLASS_BEGIN(FileManagerA)
char fname[MAX_PATH];
FILE* file;
uint8_t Mode;
errno_t err;

bool getFileBuffer(BYTE*&, size_t&) const;
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(FileManagerA, const char*, uint8_t = NET_FILE_READWRITE_APPEND)
NET_CLASS_DESTRUCTUR(FileManagerA)
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
void clear()const;
void close();
ErrorCodes getLastError() const;
FileManagerErrorRef ErrorDescription(ErrorCodes) const;
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
#endif