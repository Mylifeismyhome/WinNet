#pragma once
#include <Net/Net.h>

#define NET_FILEMANAGER Net::manager::filemanager
#define NET_FILE_BINARY std::ios::binary
#define NET_FILE_APPAND std::ios::app
#define NET_FILE_READ std::ios::in
#define NET_FILE_WRITE std::ios::out
#define NET_FILE_DISCARD std::ios::trunc
#define NET_FILE_END std::ios::ate
#define NET_FILE_READWRITE (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_END)
#define NET_FILE_READWRITE_BINARY (NET_FILE_READ | NET_FILE_WRITE | NET_FILE_BINARY | NET_FILE_END)

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(manager)
BEGIN_CLASS(filemanager)
std::string fname;
std::ifstream file;
int Mode;

bool getFileBuffer(BYTE*&, size_t&);
CLASS_PUBLIC
CLASS_CONSTRUCTUR(filemanager, const char*, int = NET_FILE_READWRITE_BINARY)
CLASS_DESTRUCTUR(filemanager)
bool openFile();
bool CanOpenFile();
bool file_exists();
bool read(BYTE*&, size_t&);
bool read(char*&);
bool write(BYTE*, size_t) const;
bool write(const char*) const;
bool write(const wchar_t*) const;
void clear()const;
END_CLASS
END_NAMESPACE
END_NAMESPACE