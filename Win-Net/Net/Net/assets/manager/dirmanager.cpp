#include "dirmanager.h"

NET_IGNORE_CONVERSION_NULL
#ifdef BUILD_LINUX
NET_FILE_ATTRW::NET_FILE_ATTRW(struct dirent* data, wchar_t* path)
{
        const size_t cSize = strlen(data->d_name) + 1;
        wchar_t* w_d_name = new wchar_t[cSize];
        mbstowcs (w_d_name, data->d_name, cSize);

        wcscpy(this->name, w_d_name);

        FREE(w_d_name);

        wcscpy(this->path, path);

        std::wstring fullPath(this->path);
        fullPath += CWSTRING("/");
        fullPath += this->name;
        wcscpy(this->fullPath, fullPath.c_str());

        std::string str_fullPath(this->fullPath[0], wcslen(this->fullPath));

        struct stat stat_buf;
        const auto rc = stat(str_fullPath.c_str(), &stat_buf);
        this->size = (rc == 0 ? stat_buf.st_size : INVALID_SIZE);

        this->lastAccess = (rc == 0 ? stat_buf.st_atime : 0);
        this->lastModification = (rc == 0 ? stat_buf.st_mtime : 0);
        this->creationTime = (rc == 0 ? stat_buf.st_ctime : 0);
}

NET_FILE_ATTRA::NET_FILE_ATTRA(struct dirent* data, char* path)
{
        strcpy(this->name, data->d_name);
        strcpy(this->path, path);

        std::string fullPath(this->path);
        fullPath += CSTRING("/");
        fullPath += this->name;
        strcpy(this->fullPath, fullPath.c_str());

        struct stat stat_buf;
        const auto rc = stat(this->fullPath, &stat_buf);
        this->size = (rc == 0 ? stat_buf.st_size : INVALID_SIZE);

        this->lastAccess = (rc == 0 ? stat_buf.st_atime : 0);
        this->lastModification = (rc == 0 ? stat_buf.st_mtime : 0);
        this->creationTime = (rc == 0 ? stat_buf.st_ctime : 0);
}
#else
static time_t filetime_to_timet(const FILETIME& ft) { ULARGE_INTEGER ull; ull.LowPart = ft.dwLowDateTime; ull.HighPart = ft.dwHighDateTime; return ull.QuadPart / 10000000ULL - 11644473600ULL; }
NET_FILE_ATTRW::NET_FILE_ATTRW(_WIN32_FIND_DATAW w32Data, wchar_t* path)
{
        wcscpy(this->name, w32Data.cFileName);
        wcscpy(this->path, path);
        
		std::wstring fullPath(this->path);
        fullPath += CWSTRING("\\");
        fullPath += this->name;
        wcscpy(this->fullPath, fullPath.c_str());

        LARGE_INTEGER fsize;
        fsize.HighPart = w32Data.nFileSizeHigh;
        fsize.LowPart = w32Data.nFileSizeLow;
        this->size = fsize.QuadPart;

        this->lastAccess = filetime_to_timet(w32Data.ftLastAccessTime);
        this->lastModification = filetime_to_timet(w32Data.ftLastWriteTime);
        this->creationTime = filetime_to_timet(w32Data.ftCreationTime);
}

NET_FILE_ATTRA::NET_FILE_ATTRA(_WIN32_FIND_DATAA w32Data, char* path)
{
        strcpy(this->name, w32Data.cFileName);
        strcpy(this->path, path);

        std::string fullPath(this->path);
        fullPath += CSTRING("\\");
        fullPath += this->name;
        strcpy(this->fullPath, fullPath.c_str());

        LARGE_INTEGER fsize;
        fsize.HighPart = w32Data.nFileSizeHigh;
        fsize.LowPart = w32Data.nFileSizeLow;
        this->size = fsize.QuadPart;

        this->lastAccess = filetime_to_timet(w32Data.ftLastAccessTime);
        this->lastModification = filetime_to_timet(w32Data.ftLastWriteTime);
        this->creationTime = filetime_to_timet(w32Data.ftCreationTime);
}
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Manager)
// Return true if the folder exists, false otherwise
bool Directory::folderExists(const wchar_t* folderName, bool ignoreHomeDir)
{
#ifdef BUILD_LINUX
	char* folderNameA = nullptr;
    wcstombs(folderNameA, folderName, wcslen(folderName));

	std::string actualPath = std::string(folderNameA);
	if(!ignoreHomeDir)
		actualPath = std::string(homeDirA() + folderNameA);

	FREE(folderNameA);

 	struct stat sb;
	if (!(stat(actualPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
		return false;
#else
#ifndef VS13
	if (_waccess(folderName, 0) == -1) {
		//File not found
		return false;
	}
#endif

	if (!(GetFileAttributesW(folderName) & FILE_ATTRIBUTE_DIRECTORY)) {
		// File is not a directory
		return false;
	}
#endif

	return true;
}

bool Directory::folderExists(const char* folderName, bool ignoreHomeDir)
{
#ifdef BUILD_LINUX
	std::string actualPath = std::string(folderNameA);
	if (!ignoreHomeDir)
		actualPath = std::string(homeDirA() + folderNameA);

	struct stat sb;
        if (!(stat(actualPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)))
                return false;
#else
#ifndef VS13
	if (_access(folderName, 0) == -1) {
		//File not found
		return false;
	}
#endif

	if (!(GetFileAttributesA(folderName) & FILE_ATTRIBUTE_DIRECTORY)) {
		// File is not a directory
		return false;
	}
#endif

	return true;
}

#ifdef BUILD_LINUX
static std::string NET_LINUX_REPLACE_ESCAPE(std::string str, const std::string& from, const std::string& to)
{
    	size_t start_pos = 0;
   	while((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
        	str.replace(start_pos, from.length(), to);
        	start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    	}
    	return str;
}
#endif

#ifdef BUILD_LINUX
static Directory::createDirResW ProcessCreateDirectory(wchar_t* path, __mode_t mode, std::vector<wchar_t*> directories = std::vector<wchar_t*>(), size_t offset = NULL)
#else
static Directory::createDirResW ProcessCreateDirectory(wchar_t* path, std::vector<wchar_t*> directories = std::vector<wchar_t*>(), size_t offset = NULL)
#endif
{
	const auto len = wcslen(path);

	// recrusive entries
	for (auto it = offset; it < len; ++it)
	{
		if (!memcmp(&path[it], CSTRING("\\"), 1))
		{
			wchar_t directory[NET_MAX_PATH];
			wcscpy(directory, &path[0]);
			directory[it] = '\0';

			if (directory[0] != '\0')
				directories.emplace_back(directory);

			offset = it + 1;
#ifdef BUILD_LINUX
			return ProcessCreateDirectory(path, mode, directories, offset);
#else
			return ProcessCreateDirectory(path, directories, offset);
#endif
		}
	}

	// last entry
	wchar_t directory[NET_MAX_PATH];
	wcscpy(directory, &path[0]);
	directory[len] = '\0';

	auto bDirectory = true;
	for (size_t it = 0; it < len; ++it)
	{
		if (!memcmp(&directory[it], CSTRING("."), 1))
		{
			bDirectory = false;
			break;
		}
	}

	if (bDirectory)
		directories.emplace_back(directory);

	std::vector<Directory::createDirResW_t> failures;
	auto bError = false;

	for (auto entry : directories)
	{
#ifdef BUILD_LINUX
		std::string actualPath(NET_DIRMANAGER::homeDirA() + std::string(entry[0], wcslen(entry)));
		actualPath = NET_LINUX_REPLACE_ESCAPE(actualPath, std::string(CSTRING("\\")), std::string(CSTRING("/")));
#endif

#ifdef BUILD_LINUX
		struct stat st = { 0 };
                if (stat(actualPath.c_str(), &st) != -1)
                {
                        failures.emplace_back(entry, Directory::createDirCodes::ERR_EXISTS);
                        bError = true;
                        continue;
                }
#else
		struct _stat st = { 0 };
		if (_wstat(entry, &st) != -1)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR_EXISTS);
			bError = true;
			continue;
		}
#endif

#ifdef BUILD_LINUX
		const auto ret = NET_WMKDIR(actualPath.c_str(), mode);
#else
		const auto ret = NET_WMKDIR(entry);
#endif

		if (ret)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR);
			bError = true;
		}
	}

	return Directory::createDirResW(bError, failures);
}

#ifdef BUILD_LINUX
static Directory::createDirResA ProcessCreateDirectory(char* path, __mode_t mode, std::vector<char*> directories = std::vector<char*>(), size_t offset = NULL)
#else
static Directory::createDirResA ProcessCreateDirectory(char* path, std::vector<char*> directories = std::vector<char*>(), size_t offset = NULL)
#endif
{
	const auto len = strlen(path);

	// recrusive entries
	for (auto it = offset; it < len; ++it)
	{
		if (!memcmp(&path[it], CSTRING("\\"), 1))
		{
			char directory[NET_MAX_PATH];
			strcpy(directory, &path[0]);
			directory[it] = '\0';

			if (directory[0] != '\0')
				directories.emplace_back(directory);

			offset = it + 1;
#ifdef BUILD_LINUX
			return ProcessCreateDirectory(path, mode, directories, offset);
#else
			return ProcessCreateDirectory(path, directories, offset);
#endif
		}
	}

	// last entry
	char directory[NET_MAX_PATH];
	strcpy(directory, &path[0]);
	directory[len] = '\0';

	auto bDirectory = true;
	for (size_t it = 0; it < len; ++it)
	{
		if (!memcmp(&directory[it], CSTRING("."), 1))
		{
			bDirectory = false;
			break;
		}
	}

	if (bDirectory)
		directories.emplace_back(directory);

	std::vector<Directory::createDirResA_t> failures;
	auto bError = false;

	for (auto entry : directories)
	{
#ifdef BUILD_LINUX
		std::string actualPath(NET_DIRMANAGER::homeDirA() + entry);
		actualPath = NET_LINUX_REPLACE_ESCAPE(actualPath, std::string(CSTRING("\\")), std::string(CSTRING("/")));
#endif

#ifndef VS13
		struct stat st = { 0 };

#ifdef BUILD_LINUX
		if (stat(actualPath.c_str(), &st) != -1)
#else
		if (stat(entry, &st) != -1)
#endif
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR_EXISTS);
			bError = true;
			continue;
		}
#endif

#ifdef BUILD_LINUX
                const auto ret = NET_MKDIR(actualPath.c_str(), mode);
#else
                const auto ret = NET_MKDIR(entry);
#endif

		if (ret)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR);
			bError = true;
		}
	}

	return Directory::createDirResA(bError, failures);
}

#ifdef BUILD_LINUX
Directory::createDirResW Directory::createDir(wchar_t* path, __mode_t mode)
#else
Directory::createDirResW Directory::createDir(wchar_t* path)
#endif
{
	const auto len = wcslen(path);

	wchar_t fixed[NET_MAX_PATH];
	size_t flen = NULL;
	for (size_t it = 0; it < len; ++it)
	{
		if (wmemcmp(&path[it], CWSTRING("//"), 2) != 0
			&& wmemcmp(&path[it], CWSTRING("\\\\"), 2) != 0)
		{
			memcpy(&fixed[flen], &path[it], 1);
			flen++;
		}
	}
	for (size_t it = 0; it < flen; ++it)
	{
		if (fixed[it] == '/')
			fixed[it] = '\\';
	}
	fixed[flen] = '\0';

#ifdef BUILD_LINUX
	return ProcessCreateDirectory(fixed, mode);
#else
	return ProcessCreateDirectory(fixed);
#endif
}

#ifdef BUILD_LINUX
Directory::createDirResA Directory::createDir(char* path, __mode_t mode)
#else
Directory::createDirResA Directory::createDir(char* path)
#endif
{
	const auto len = strlen(path);

	char fixed[NET_MAX_PATH];
	size_t flen = NULL;
	for (size_t it = 0; it < len; ++it)
	{
		if (memcmp(&path[it], CSTRING("//"), 2) != 0
			&& memcmp(&path[it], CSTRING("\\\\"), 2) != 0)
		{
			memcpy(&fixed[flen], &path[it], 1);
			flen++;
		}
	}
	for (size_t it = 0; it < flen; ++it)
	{
		if (fixed[it] == '/')
			fixed[it] = '\\';
	}
	fixed[flen] = '\0';

#ifdef BUILD_LINUX
	return ProcessCreateDirectory(fixed, mode);
#else
	return ProcessCreateDirectory(fixed);
#endif
}

#ifdef BUILD_LINUX
static int NET_LINUX_IS_DIRECTORY(const char* path)
{
    struct stat s_buf;

    if (stat(path, &s_buf))
        return 0;

    return S_ISDIR(s_buf.st_mode);
}
#endif

#ifdef BUILD_LINUX
bool Directory::deleteDir(wchar_t* dirname)
#else
bool Directory::deleteDir(wchar_t* dirname, const bool bDeleteSubdirectories, bool ignoreHomeDir)
#endif
{
#ifdef BUILD_LINUX
	std::string dirnameA(dirname[0], wcslen(dirname));
	std::string tmpPath(dirnameA);
        tmpPath = NET_LINUX_REPLACE_ESCAPE(tmpPath, CSTRING("\\"), CSTRING("/"));
        tmpPath = NET_LINUX_REPLACE_ESCAPE(tmpPath, CSTRING("//"), CSTRING("/"));

        std::vector<std::string> vPath;

        bool bNeedInsertLastCase = true;
        for(size_t i = 0; i < tmpPath.size(); ++i)
        {
                if(tmpPath[i] == '/')
                {
			if((i + 1) == tmpPath.size()) bNeedInsertLastCase = false;
                        vPath.emplace_back(tmpPath.substr(0, i));
                }
        }

        // last case
        if(bNeedInsertLastCase) vPath.emplace_back(tmpPath);

        for(auto it = vPath.rbegin(); it != vPath.rend(); it++)
        {
			std::string str((*it));
			if (!ignoreHomeDir)
				str = std::string(homeDirA() + (*it));

			rmdir(str.c_str());
        }
#else
	std::wstring     strFilePath;                 // Filepath
	std::wstring     strPattern;                  // Pattern
	WIN32_FIND_DATAW FileInformation;             // File information

	std::wstring str(dirname);
	if (!ignoreHomeDir)
		str = std::wstring(homeDirW() + dirname);

	strPattern = str + CWSTRING("\\*.*");
	const auto hFile = ::FindFirstFileW(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = str + CWSTRING("\\") + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						const auto iRC = deleteDir((wchar_t*)strFilePath.data(), bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributesW(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFileW(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFileW(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		const auto dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bDeleteSubdirectories)
			{
				// Set directory attributes
				if (::SetFileAttributesW(dirname,
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectoryW(dirname) == FALSE)
					return ::GetLastError();
			}
		}
	}

#endif
	return true;
}

#ifdef BUILD_LINUX
bool Directory::deleteDir(char* dirname)
#else
bool Directory::deleteDir(char* dirname, const bool bDeleteSubdirectories, bool ignoreHomeDir)
#endif
{
#ifdef BUILD_LINUX
	std::string tmpPath(dirname);
        tmpPath = NET_LINUX_REPLACE_ESCAPE(tmpPath, CSTRING("\\"), CSTRING("/"));
	tmpPath = NET_LINUX_REPLACE_ESCAPE(tmpPath, CSTRING("//"), CSTRING("/"));

        std::vector<std::string> vPath;

        bool bNeedInsertLastCase = true;
        for(size_t i = 0; i < tmpPath.size(); ++i)
        {
                if(tmpPath[i] == '/')
                {
                        if((i + 1) == tmpPath.size()) bNeedInsertLastCase = false;
                        vPath.emplace_back(tmpPath.substr(0, i));
                }
        }

        // last case
        if(bNeedInsertLastCase) vPath.emplace_back(tmpPath);

        for(auto it = vPath.rbegin(); it != vPath.rend(); it++)
        {
			std::string str((*it));
			if (!ignoreHomeDir)
				str = std::string(homeDirA() + (*it));

			rmdir(str.c_str());
        }
#else
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	std::string str(dirname);
	if (!ignoreHomeDir)
		str = std::string(homeDirA() + dirname);

	strPattern = str + CSTRING("\\*.*");
	const auto hFile = ::FindFirstFileA(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = str + CSTRING("\\") + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						const auto iRC = deleteDir((char*)strFilePath.data(), bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributesA((LPCSTR)strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFileA((LPCSTR)strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFileA(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		const auto dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bDeleteSubdirectories)
			{
				// Set directory attributes
				if (::SetFileAttributesA(dirname,
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectoryA(dirname) == FALSE)
					return ::GetLastError();
			}
		}
	}
#endif

	return true;
}

void Directory::scandir(wchar_t* Dirname, std::vector<NET_FILE_ATTRW>& Vector, bool ignoreHomeDir)
{
	std::string actualDirname(std::string(Dirname[0], wcslen(Dirname)));
	std::wstring WactualDirname(Dirname);

	if (!ignoreHomeDir)
	{
		actualDirname = std::string(homeDirA() + std::string(Dirname[0], wcslen(Dirname)));
		WactualDirname = std::wstring(homeDirW() + Dirname);
	}
	
#ifdef BUILD_LINUX
	const auto dir = opendir(actualDirname.c_str());
        if(!dir) return;
        struct dirent* entry;
        while ((entry = readdir(dir)))
        {
                if(!strcmp(entry->d_name, CSTRING(".")) || !strcmp(entry->d_name, CSTRING(".."))) continue;

                // iterate recursive
                if(entry->d_type == DT_DIR)
                {
                        std::wstring nextDir(Dirname);
                        nextDir += CWSTRING("/");

 			const size_t cSize = strlen(entry->d_name) + 1;
        		wchar_t* w_d_name = new wchar_t[cSize];
        		mbstowcs (w_d_name, entry->d_name, cSize);

        		nextDir += w_d_name;

       			FREE(w_d_name);

                        scandir((wchar_t*)nextDir.c_str(), Vector, ignoreHomeDir);
                        continue;
                }

                Vector.emplace_back(NET_FILE_ATTRW(entry, (wchar_t*)WactualDirname.c_str()));
        }
        closedir(dir);
#else
	WIN32_FIND_DATAW ffblk;
	wchar_t buf[NET_MAX_PATH];

	if (WactualDirname.empty())
		swprintf_s(buf, CWSTRING("%s"), CWSTRING("*.*"));
	else
		swprintf_s(buf, CWSTRING("%s\\%s"), WactualDirname.c_str(), CWSTRING("*.*"));

	const auto hFind = FindFirstFileW(buf, &ffblk);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do
	{
		if (WactualDirname.empty())
			swprintf_s(buf, CWSTRING("%s"), ffblk.cFileName);
		else
			swprintf_s(buf, CWSTRING("%s\\%s"), Dirname, ffblk.cFileName);

		if (wcscmp(reinterpret_cast<const wchar_t*>(ffblk.cFileName), CWSTRING(".")) == 0 || wcscmp(reinterpret_cast<const wchar_t*>(ffblk.cFileName), CWSTRING("..")) == 0)
			continue;

		const auto isDir = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		// recurse to directory
		if (isDir)
			scandir(buf, Vector, ignoreHomeDir);
		else
			Vector.emplace_back(NET_FILE_ATTRW(ffblk, (wchar_t*)WactualDirname.c_str()));

	} while (FindNextFileW(hFind, &ffblk) != 0);

	FindClose(hFind);

	memset(buf, NULL, NET_MAX_PATH);
#endif
}

void Directory::scandir(char* Dirname, std::vector<NET_FILE_ATTRA>& Vector, bool ignoreHomeDir)
{
	std::string actualDirname(Dirname);

	if (!ignoreHomeDir)
		actualDirname = std::string(homeDirA() + Dirname);

#ifdef BUILD_LINUX
	const auto dir = opendir(actualDirname.c_str());
        if(!dir) return;
        struct dirent* entry;
        while ((entry = readdir(dir)))
        {
                if(!strcmp(entry->d_name, CSTRING(".")) || !strcmp(entry->d_name, CSTRING(".."))) continue;

                // iterate recursive
                if(entry->d_type == DT_DIR)
                {
                        std::string nextDir(Dirname);
                        nextDir += CSTRING("/");
                        nextDir += entry->d_name;
                        scandir((char*)nextDir.c_str(), Vector, ignoreHomeDir);
                        continue;
                }

		Vector.emplace_back(NET_FILE_ATTRA(entry, (char*)actualDirname.c_str()));
        }
        closedir(dir);
#else
	WIN32_FIND_DATA ffblk;
	char buf[NET_MAX_PATH];

	if (actualDirname.empty())
		sprintf_s(buf, CSTRING("%s"), CSTRING("*.*"));
	else
		sprintf_s(buf, CSTRING("%s\\%s"), actualDirname.c_str(), CSTRING("*.*"));

	const auto hFind = FindFirstFileA((LPCSTR)buf, &ffblk);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do
	{
		if (actualDirname.empty())
			sprintf_s(buf, CSTRING("%s"), ffblk.cFileName);
		else
			sprintf_s(buf, CSTRING("%s\\%s"), Dirname, ffblk.cFileName);

		if (strcmp(reinterpret_cast<const char*>(ffblk.cFileName), CSTRING(".")) == 0 || strcmp(reinterpret_cast<const char*>(ffblk.cFileName), CSTRING("..")) == 0)
			continue;

		const auto isDir = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		// recurse to directory
		if (isDir)
			scandir(buf, Vector, ignoreHomeDir);
		else
			Vector.emplace_back(NET_FILE_ATTRA(ffblk, (char*)actualDirname.c_str()));

	} while (FindNextFile(hFind, &ffblk) != 0);

	FindClose(hFind);

	memset(buf, NULL, NET_MAX_PATH);
#endif
}

std::wstring Directory::homeDirW()
{
#ifdef BUILD_LINUX
	char result[NET_MAX_PATH];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, NET_MAX_PATH);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        str = str.substr(0, it);
	return std::wstring(str.begin(), str.end());
#else
	do
	{
		wchar_t result[NET_MAX_PATH];
		const auto size = GetModuleFileNameW(nullptr, result, NET_MAX_PATH);
		if (!size)
			continue;

		const std::wstring tmp(result, size);
		const auto f = tmp.find_last_of('\\');
		if (f != std::wstring::npos)
		{
			const auto sub = tmp.substr(0, f + 1);
			return std::wstring(sub);
		}

		return std::wstring(result, size);
	} while (true);
#endif
}

std::string Directory::homeDirA()
{
#ifdef BUILD_LINUX
	char result[NET_MAX_PATH];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, NET_MAX_PATH);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        return str.substr(0, it);
#else
	do
	{
		char result[NET_MAX_PATH];
		const auto size = GetModuleFileNameA(nullptr, result, NET_MAX_PATH);
		if (!size)
			continue;

		const std::string tmp(result, size);
		const auto f = tmp.find_last_of('\\');
		if (f != std::string::npos)
		{
			const auto sub = tmp.substr(0, f + 1);
			return std::string(sub);
		}

		return std::string(result, size);
	} while (true);
#endif
}

std::wstring Directory::currentFileNameW()
{
#ifdef BUILD_LINUX
	char result[NET_MAX_PATH];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, NET_MAX_PATH);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        str = str.substr(it, str.size() - it);
	return std::wstring(str.begin(), str.end());
#else
	do
	{
		wchar_t result[NET_MAX_PATH];
		const auto size = GetModuleFileNameW(nullptr, result, NET_MAX_PATH);
		if (!size)
			continue;

		const std::wstring tmp(result, size);
		const auto f = tmp.find_last_of('\\');
		if (f != std::wstring::npos)
		{
			const auto sub = tmp.substr(f + 1);
			return std::wstring(sub);
		}

		return std::wstring(result, size);
	} while (true);
#endif
}

std::string Directory::currentFileNameA()
{
#ifdef BUILD_LINUX
	char result[NET_MAX_PATH];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, NET_MAX_PATH);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        return str.substr(it, str.size() - it);
#else
	do
	{
		char result[NET_MAX_PATH];
		const auto size = GetModuleFileNameA(nullptr, result, NET_MAX_PATH);
		if (!size)
			continue;

		const std::string tmp(result, size);
		const auto f = tmp.find_last_of('\\');
		if (f != std::string::npos)
		{
			const auto sub = tmp.substr(f + 1);
			return std::string(sub);
		}

		return std::string(result, size);
	} while (true);
#endif
}
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_POP
