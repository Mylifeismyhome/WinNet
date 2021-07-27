#pragma once
#include <Net/Net/Net.h>
#include <Net/assets/manager/logmanager.h>

#ifdef BUILD_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#else
#include <list>

#ifndef VS13
#include <corecrt_io.h>
#endif

#include <direct.h>
#endif

NET_DSA_BEGIN

#ifdef UNICODE
#define NET_FILE_ATTR_ NET_FILE_ATTRW

#define homeDir homeDirW
#define currentFileName currentFileNameW
#else
#define NET_FILE_ATTR_ NET_FILE_ATTRA

#define homeDir homeDirA
#define currentFileName currentFileNameA
#endif
#define NET_FILE_ATTR std::vector<NET_FILE_ATTR_>

#define NET_FILES NET_FILE_ATTR

#define NET_DIRMANAGER Net::Manager::Directory
#define NET_CREATEDIR Net::Manager::Directory::createDir
#define NET_DELETEDIR Net::Manager::Directory::deleteDir
#define NET_SCANDIR Net::Manager::Directory::scandir
#define NET_HOMEDIR Net::Manager::Directory::homeDir()
#define NET_CURRENTFILENAME(x) Net::Manager::Directory::currentFileName(x)

#ifdef BUILD_LINUX
#define NET_MAX_PATH PATH_MAX
#define NET_MKDIR mkdir
#define NET_WMKDIR NET_MKDIR
#define NET_DEFAULT_MKDIR_MODE (/*OWNER*/(S_IRUSR | S_IWUSR | S_IXUSR) | /*GROUP*/(S_IRGRP | S_IXGRP) | /*OTHER*/(S_IROTH | S_IXOTH))
#else
#define NET_MAX_PATH MAX_PATH
#define NET_MKDIR _mkdir
#define NET_WMKDIR _wmkdir
#endif

struct NET_FILE_ATTRW
{
        wchar_t name[NET_MAX_PATH];
        wchar_t path[NET_MAX_PATH];
        wchar_t fullPath[NET_MAX_PATH];

        size_t size;
        time_t lastAccess;
        time_t lastModification;
        time_t creationTime;

	#ifdef BUILD_LINUX
	NET_FILE_ATTRW(struct dirent*, wchar_t*);
	#else
        _WIN32_FIND_DATAW w32Data;

        NET_FILE_ATTRW(_WIN32_FIND_DATAW, wchar_t*);
        #endif
};

struct NET_FILE_ATTRA
{
        char name[NET_MAX_PATH];
        char path[NET_MAX_PATH];
        char fullPath[NET_MAX_PATH];

        size_t size;
        time_t lastAccess;
        time_t lastModification;
        time_t creationTime;

        #ifdef BUILD_LINUX
	NET_FILE_ATTRA(struct dirent*, char*);
	#else
	_WIN32_FIND_DATAA w32Data;

        NET_FILE_ATTRA(_WIN32_FIND_DATAA, char*);
        #endif
};

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Manager)
NET_NAMESPACE_BEGIN(Directory)
enum class createDirCodes
{
	SUCCESS = 0,
	ERR,
	ERR_EXISTS
};

struct createDirResW_t
{
	wchar_t entry[NET_MAX_PATH];
	int code;

	createDirResW_t(const wchar_t* entry, const createDirCodes code)
	{
		wcscpy(this->entry, entry);
		this->code = (int)code;
	}
};

struct createDirResW
{
	bool error;
	std::vector<createDirResW_t> failures;

	createDirResW(const bool error, const std::vector<createDirResW_t>& failures)
	{
		this->error = error;
		this->failures = failures;
	}
};

struct createDirResA_t
{
	char entry[NET_MAX_PATH];
	int code;

	createDirResA_t(const char* entry, const createDirCodes code)
	{
		strcpy(this->entry, entry);
		this->code = (int)code;
	}
};

struct createDirResA
{
	bool error;
	std::vector<createDirResA_t> failures;

	createDirResA(const bool error, const std::vector<createDirResA_t>& failures)
	{
		this->error = error;
		this->failures = failures;
	}
};

bool folderExists(const wchar_t*);
bool folderExists(const char*);

#ifdef BUILD_LINUX
createDirResW createDir(wchar_t*, __mode_t = NET_DEFAULT_MKDIR_MODE);
createDirResA createDir(char*, __mode_t = NET_DEFAULT_MKDIR_MODE);
#else
createDirResW createDir(wchar_t*);
createDirResA createDir(char*);
#endif

#ifdef BUILD_LINUX
bool deleteDir(wchar_t*);
bool deleteDir(char*);
#else
bool deleteDir(wchar_t*, bool = true);
bool deleteDir(char*, bool = true);
#endif

void scandir(wchar_t*, std::vector<NET_FILE_ATTRW>&);
void scandir(char*, std::vector<NET_FILE_ATTRA>&);
std::wstring homeDirW();
std::string homeDirA();
std::wstring currentFileNameW();
std::string currentFileNameA();
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
