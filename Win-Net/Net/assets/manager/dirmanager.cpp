#include "dirmanager.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(manager)
// Return true if the folder exists, false otherwise
bool dirmanager::folderExists(const char* folderName) {
	if (_access(folderName, 0) == -1) {
		//File not found
		return false;
	}

	if (!(GetFileAttributes((LPCSTR)folderName) & FILE_ATTRIBUTE_DIRECTORY)) {
		// File is not a directory
		return false;
	}

	return true;
}

static dirmanager::createDirRes ProcessCreateDirectory(char* path, std::vector<char*> directories = std::vector<char*>(), size_t offset = NULL)
{
	const auto len = strlen(path);

	// recrusive entries
	for (auto it = offset; it < len; ++it)
	{
		if (!memcmp(&path[it], CSTRING("/"), 1))
		{
			char directory[MAX_PATH];
			strcpy_s(directory, MAX_PATH, &path[offset]);
			directory[it - offset] = '\0';

			if (directory[0] != '\0')
				directories.emplace_back(directory);

			offset = it + 1;
			return ProcessCreateDirectory(path, directories, offset);
		}
	}

	// last entry
	char directory[MAX_PATH];
	strcpy_s(directory, MAX_PATH, &path[offset]);
	directory[len] = '\0';

	auto bDirectory = true;
	for (size_t it = 0; it < len; ++it)
	{
		if (!memcmp(&directory[it], ".", 1))
		{
			bDirectory = false;
			break;
		}
	}

	if (bDirectory)
		directories.emplace_back(directory);

	for (auto entry : directories)
	{
		if (!CreateDirectoryA(entry, nullptr))
			return dirmanager::createDirRes::ERR;

		if (_chdir(entry) != 0)
			return dirmanager::createDirRes::CAN_NOT_CHANGE_DIR;
	}

	for (size_t it = 0; it < directories.size(); ++it)
	{
		if (_chdir(CSTRING("..")) != 0)
			return dirmanager::createDirRes::CAN_NOT_CHANGE_DIR;
	}

	return dirmanager::createDirRes::SUCCESS;
}

dirmanager::createDirRes dirmanager::createDir(char* path)
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
		if (fixed[it] == '\\')
			fixed[it] = '/';
	}
	fixed[flen] = '\0';
	return ProcessCreateDirectory(fixed);
}

bool dirmanager::deleteDir(char* dirname, const bool bDeleteSubdirectories)
{
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	const std::string str(dirname);
	strPattern = str + CSTRING("\\*.*");
	const auto hFile = ::FindFirstFile((LPCSTR)strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = str + CSTRING("\\") + (const char*)FileInformation.cFileName;

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
					if (::SetFileAttributes((LPCSTR)strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile((LPCSTR)strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

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
				if (::SetFileAttributes((LPCSTR)dirname,
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory((LPCSTR)dirname) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return true;
}

void dirmanager::scandir(char* Dirname, std::vector<NET_FILE_ATTR>& Vector)
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
			Vector.emplace_back(NET_FILE_ATTR(ffblk, buf));

	} while (FindNextFile(hFind, &ffblk) != 0);

	FindClose(hFind);

	memset(buf, NULL, MAX_PATH);
}

std::string dirmanager::currentDir()
{
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
}

std::string dirmanager::currentFileName()
{
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
}
NET_NAMESPACE_END
NET_NAMESPACE_END
