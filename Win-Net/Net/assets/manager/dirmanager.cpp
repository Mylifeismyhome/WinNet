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

// Returns false on success, true on error
bool dirmanager::createFolderTree(const char* path)
{
	const std::string folderName = path;

	std::list<std::string> folderLevels;
	const auto c_str = (char*)folderName.c_str();

	// Point to end of the string
	auto strPtr = &c_str[strlen(c_str) - 1];

	// Create a list of the folders which do not currently exist
	do {
		if (folderExists(c_str)) {
			break;
		}
		// Break off the last folder name, store in folderLevels list
		do {
			strPtr--;
		} while ((*strPtr != '\\') && (*strPtr != '/') && (strPtr >= c_str));
		folderLevels.push_front(std::string(strPtr + 1));
		strPtr[1] = 0;
	} while (strPtr >= c_str);

	if (_chdir(c_str)) {
		return true;
	}

	// Create the folders iteratively
	for (const auto& entry : folderLevels) {
		if (CreateDirectory(entry.c_str(), nullptr) == 0) {
			return true;
		}
		_chdir(entry.c_str());
	}

	return false;
}

bool dirmanager::createDir(char* Dirname)
{
	return CreateDirectory(Dirname, nullptr);
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
