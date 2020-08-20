#pragma once
#include <Net/Net.h>

NET_DSA_BEGIN

#define NET_FILEMANAGER Net::manager::filemanager
#define NET_FILE_BINARY std::ios::binary
#define NET_FILE_APPAND std::ios::app
#define NET_FILE_READ std::ios::in
#define NET_FILE_WRITE std::ios::out
#define NET_FILE_DISCARD std::ios::trunc
#define NET_FILE_END std::ios::ate
#define NET_FILE_READWRITE (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_END)
#define NET_FILE_READWRITE_BINARY (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_BINARY | NET_FILE_END)

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
NET_CLASS_BEGIN(filemanager)
std::string fname;
std::ifstream file;
int Mode;

bool getFileBuffer(BYTE*&, size_t&);
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(filemanager, const char*, int = NET_FILE_READWRITE_BINARY)
NET_CLASS_DESTRUCTUR(filemanager)
bool openFile();
bool CanOpenFile();
bool file_exists();
bool read(BYTE*&, size_t&);
bool read(char*&);
bool write(BYTE*, size_t) const;
bool write(const char*) const;
bool write(const wchar_t*) const;
void clear()const;
void close();
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END