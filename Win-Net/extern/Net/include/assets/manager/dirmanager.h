#pragma once
#include <Net/Net.h>
#include <assets/manager/logmanager.h>
#include <list>
#include <corecrt_io.h>
#include <direct.h>

NET_DSA_BEGIN

struct NET_FILE_ATTRW
{
	_WIN32_FIND_DATAW w32Data;
	wchar_t path[MAX_PATH];

	NET_FILE_ATTRW(const _WIN32_FIND_DATAW w32Data, const wchar_t* path)
	{
		this->w32Data = w32Data;
		wcscpy_s(this->path, path);
	}
};

struct NET_FILE_ATTRA
{
	_WIN32_FIND_DATAA w32Data;
	char path[MAX_PATH];

	NET_FILE_ATTRA(const _WIN32_FIND_DATAA w32Data, const char* path)
	{
		this->w32Data = w32Data;
		strcpy_s(this->path, path);
	}
};

#ifdef UNICODE
#define NET_FILE_ATTR_ NET_FILE_ATTRW

#define currentDir currentDirW
#define currentFileName currentFileNameW
#else
#define NET_FILE_ATTR_ NET_FILE_ATTRA

#define currentDir currentDirA
#define currentFileName currentFileNameA
#endif
#define NET_FILE_ATTR std::vector<NET_FILE_ATTR_>

#define NET_FILES NET_FILE_ATTR

#define NET_DIRMANAGER Net::manager::dirmanager
#define NET_CREATEDIR Net::manager::dirmanager::createDir
#define NET_DELETEDIR Net::manager::dirmanager::deleteDir
#define NET_SCANDIR Net::manager::dirmanager::scandir
#define NET_CURRENTDIR Net::manager::dirmanager::currentDir()
#define NET_CURRENTFILENAME(x) Net::manager::dirmanager::currentFileName(x)

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
NET_NAMESPACE_BEGIN(dirmanager)
enum class createDirRes
{
	SUCCESS = 0,
	ERR,
	CAN_NOT_CHANGE_DIR
};

bool folderExists(const wchar_t*);
bool folderExists(const char*);
createDirRes createDir(wchar_t*);
createDirRes createDir(char*);
bool deleteDir(wchar_t*, bool = true);
bool deleteDir(char*, bool = true);
void scandir(wchar_t*, std::vector<NET_FILE_ATTRW>&);
void scandir(char*, std::vector<NET_FILE_ATTRA>&);
std::wstring currentDirW();
std::string currentDirA();
std::wstring currentFileNameW();
std::string currentFileNameA();
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END