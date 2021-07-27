#include "dirmanager.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Manager)
// Return true if the folder exists, false otherwise
bool Directory::folderExists(const wchar_t* folderName)
{
#ifdef BUILD_LINUX
	char* folderNameA = nullptr;
        wcstombs(folderNameA, folderName, wcslen(folderName));

	auto actualPath = homeDirA();
	actualPath += folderNameA;
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

bool Directory::folderExists(const char* folderName)
{
#ifdef BUILD_LINUX
 	auto actualPath = homeDirA();
        actualPath += folderName;

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

#ifndef BUILD_LINUX
static Directory::createDirResW ProcessCreateDirectory(wchar_t* path, std::vector<wchar_t*> directories = std::vector<wchar_t*>(), size_t offset = NULL)
{
	const auto len = wcslen(path);

	// recrusive entries
	for (auto it = offset; it < len; ++it)
	{
		if (!memcmp(&path[it], CSTRING("\\"), 1))
		{
			wchar_t directory[MAX_PATH];
			wcscpy_s(directory, MAX_PATH, &path[0]);
			directory[it] = '\0';

			if (directory[0] != '\0')
				directories.emplace_back(directory);

			offset = it + 1;
			return ProcessCreateDirectory(path, directories, offset);
		}
	}

	// last entry
	wchar_t directory[MAX_PATH];
	wcscpy_s(directory, MAX_PATH, &path[0]);
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
		struct _stat st = { 0 };
		if (_wstat(entry, &st) != -1)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR_EXISTS);
			bError = true;
			continue;
		}

		const auto ret = _wmkdir(entry);
		if (ret)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR);
			bError = true;
		}
	}

	return Directory::createDirResW(bError, failures);
}

static Directory::createDirResA ProcessCreateDirectory(char* path, std::vector<char*> directories = std::vector<char*>(), size_t offset = NULL)
{
	const auto len = strlen(path);

	// recrusive entries
	for (auto it = offset; it < len; ++it)
	{
		if (!memcmp(&path[it], CSTRING("\\"), 1))
		{
			char directory[MAX_PATH];
			strcpy_s(directory, MAX_PATH, &path[0]);
			directory[it] = '\0';

			if (directory[0] != '\0')
				directories.emplace_back(directory);

			offset = it + 1;
			return ProcessCreateDirectory(path, directories, offset);
		}
	}

	// last entry
	char directory[MAX_PATH];
	strcpy_s(directory, MAX_PATH, &path[0]);
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
#ifndef VS13
		struct stat st = { 0 };
		if (stat(entry, &st) != -1)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR_EXISTS);
			bError = true;
			continue;
		}
#endif

		const auto ret = _mkdir(entry);
		if (ret)
		{
			failures.emplace_back(entry, Directory::createDirCodes::ERR);
			bError = true;
		}
	}

	return Directory::createDirResA(bError, failures);
}

Directory::createDirResW Directory::createDir(wchar_t* path)
{
	const auto len = wcslen(path);

	wchar_t fixed[MAX_PATH];
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
	return ProcessCreateDirectory(fixed);
}

Directory::createDirResA Directory::createDir(char* path)
{
	const auto len = strlen(path);

	char fixed[MAX_PATH];
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
	return ProcessCreateDirectory(fixed);
}

bool Directory::deleteDir(wchar_t* dirname, const bool bDeleteSubdirectories)
{
	std::wstring     strFilePath;                 // Filepath
	std::wstring     strPattern;                  // Pattern
	WIN32_FIND_DATAW FileInformation;             // File information

	const std::wstring str(dirname);
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

	return true;
}

bool Directory::deleteDir(char* dirname, const bool bDeleteSubdirectories)
{
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	const std::string str(dirname);
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

	return true;
}

void Directory::scandir(wchar_t* Dirname, std::vector<NET_FILE_ATTRW>& Vector)
{
	WIN32_FIND_DATAW ffblk;
	wchar_t buf[MAX_PATH];

	if (!Dirname)
		swprintf_s(buf, CWSTRING("%s"), CWSTRING("*.*"));
	else
		swprintf_s(buf, CWSTRING("%s\\%s"), Dirname, CWSTRING("*.*"));

	const auto hFind = FindFirstFileW(buf, &ffblk);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do
	{
		if (!Dirname)
			swprintf_s(buf, CWSTRING("%s"), ffblk.cFileName);
		else
			swprintf_s(buf, CWSTRING("%s\\%s"), Dirname, ffblk.cFileName);

		if (wcscmp(reinterpret_cast<const wchar_t*>(ffblk.cFileName), CWSTRING(".")) == 0 || wcscmp(reinterpret_cast<const wchar_t*>(ffblk.cFileName), CWSTRING("..")) == 0)
			continue;

		const auto isDir = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		// recurse to directory
		if (isDir)
			scandir(buf, Vector);
		else
			Vector.emplace_back(NET_FILE_ATTRW(ffblk, buf));

	} while (FindNextFileW(hFind, &ffblk) != 0);

	FindClose(hFind);

	memset(buf, NULL, MAX_PATH);
}

void Directory::scandir(char* Dirname, std::vector<NET_FILE_ATTRA>& Vector)
{
	WIN32_FIND_DATA ffblk;
	char buf[MAX_PATH];

	if (!Dirname)
		sprintf_s(buf, CSTRING("%s"), CSTRING("*.*"));
	else
		sprintf_s(buf, CSTRING("%s\\%s"), Dirname, CSTRING("*.*"));

	const auto hFind = FindFirstFile((LPCSTR)buf, &ffblk);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do
	{
		if (!Dirname)
			sprintf_s(buf, CSTRING("%s"), ffblk.cFileName);
		else
			sprintf_s(buf, CSTRING("%s\\%s"), Dirname, ffblk.cFileName);

		if (strcmp(reinterpret_cast<const char*>(ffblk.cFileName), CSTRING(".")) == 0 || strcmp(reinterpret_cast<const char*>(ffblk.cFileName), CSTRING("..")) == 0)
			continue;

		const auto isDir = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		// recurse to directory
		if (isDir)
			scandir(buf, Vector);
		else
			Vector.emplace_back(NET_FILE_ATTRA(ffblk, buf));

	} while (FindNextFile(hFind, &ffblk) != 0);

	FindClose(hFind);

	memset(buf, NULL, MAX_PATH);
}
#endif

std::wstring Directory::homeDirW()
{
#ifdef BUILD_LINUX
	char result[PATH_MAX];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, PATH_MAX);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        str = str.substr(0, it);
	return std::wstring(str.begin(), str.end());
#else
	do
	{
		wchar_t result[MAX_PATH];
		const auto size = GetModuleFileNameW(nullptr, result, MAX_PATH);
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
	char result[PATH_MAX];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, PATH_MAX);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        return str.substr(0, it);
#else
	do
	{
		char result[MAX_PATH];
		const auto size = GetModuleFileNameA(nullptr, result, MAX_PATH);
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
	char result[PATH_MAX];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, PATH_MAX);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        str = str.substr(it, str.size() - it);
	return std::wstring(str.begin(), str.end());
#else
	do
	{
		wchar_t result[MAX_PATH];
		const auto size = GetModuleFileNameW(nullptr, result, MAX_PATH);
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
	char result[PATH_MAX];
        ssize_t count = readlink(CSTRING("/proc/self/exe"), result, PATH_MAX);
        auto str = std::string(result, (count > 0) ? count : 0);
        const auto it = str.find_last_of(CSTRING("/")) + 1;
        return str.substr(it, str.size() - it);
#else
	do
	{
		char result[MAX_PATH];
		const auto size = GetModuleFileNameA(nullptr, result, MAX_PATH);
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
