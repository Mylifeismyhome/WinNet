#pragma once
#include <Net/Net.h>
#include <assets/manager/logmanager.h>
#include <list>
#include <corecrt_io.h>
#include <direct.h>

NET_DSA_BEGIN

struct NET_FILE_ATTR
{
	_WIN32_FIND_DATAA w32Data;
	char path[MAX_PATH];

	NET_FILE_ATTR(const _WIN32_FIND_DATAA w32Data, const char* path)
	{
		this->w32Data = w32Data;
		strcpy_s(this->path, path);
	}
};

#define NET_FILES std::vector<NET_FILE_ATTR>

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

extern "C" NET_API bool folderExists(const char*);
extern "C" NET_API createDirRes createDir(char*);
extern "C" NET_API bool deleteDir(char*, bool = true);
extern "C" NET_API void scandir(char*, std::vector<NET_FILE_ATTR>&);
std::string currentDir();
std::string currentFileName();
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END