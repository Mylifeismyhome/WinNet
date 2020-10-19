#pragma once
#include <Net/Net.h>
#include "Cryption/XOR.h"

NET_DSA_BEGIN

#ifdef UNICODE
#define NET_FILEMANAGER Net::manager::FileManagerW
#else
#define NET_FILEMANAGER Net::manager::FileManagerA
#endif

#define NET_FILEMANAGERW Net::manager::FileManagerW
#define NET_FILEMANAGERA Net::manager::FileManagerA

#define NET_FILE_APPAND (1 << 0)
#define NET_FILE_READ (1 << 1)
#define NET_FILE_WRITE (1 << 2)
#define NET_FILE_DISCARD (1 << 3)
#define NET_FILE_READWRITE (NET_FILE_READ | NET_FILE_WRITE)
#define NET_FILE_READWRITE_APPEND (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_APPAND)

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
NET_CLASS_BEGIN(FileManagerW)
wchar_t fname[MAX_PATH];
FILE* file;
uint8_t Mode;

bool getFileBuffer(BYTE*&, size_t&);
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
void clear()const;
void close();
NET_CLASS_END
NET_CLASS_BEGIN(FileManagerA)
char fname[MAX_PATH];
FILE* file;
uint8_t Mode;

bool getFileBuffer(BYTE*&, size_t&);
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
void clear()const;
void close();
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END