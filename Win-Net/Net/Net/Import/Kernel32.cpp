#define MODULE_NAME CSTRING("Kernel32")

#include "Kernel32.h"
#include <Net/Cryption/PointerCryption.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/Import/MemoryModule.h>

namespace Net
{
	IMPORT_BEGIN(Kernel32)
		IMPORT_HANDLE(HMEMORYMODULE, handle);

	IMPORT_DEFINE(FindResourceA);
	IMPORT_DEFINE(FindResourceW);
	IMPORT_DEFINE(LoadResource);
	IMPORT_DEFINE(SizeofResource);
	IMPORT_DEFINE(LockResource);
	IMPORT_DEFINE(LoadLibraryA);
	IMPORT_DEFINE(LoadLibraryW);
	IMPORT_DEFINE(GetProcAddress);
	IMPORT_DEFINE(FreeLibrary);
	IMPORT_DEFINE(Sleep);
	IMPORT_DEFINE(K32EnumProcesses);
	IMPORT_DEFINE(OpenProcess);
	IMPORT_DEFINE(K32EnumProcessModules);
	IMPORT_DEFINE(K32GetModuleBaseNameA);
	IMPORT_DEFINE(CheckRemoteDebuggerPresent);
	IMPORT_DEFINE(IsDebuggerPresent);
	IMPORT_DEFINE(GetVersionExA);
	IMPORT_DEFINE(GetModuleHandleA);
	IMPORT_DEFINE(GetModuleFileNameA);
	IMPORT_DEFINE(MapViewOfFile);
	IMPORT_DEFINE(CreateFileMappingA);
	IMPORT_DEFINE(CreateFileA);
	IMPORT_DEFINE(UnmapViewOfFile);
	IMPORT_DEFINE(CloseHandle);
	IMPORT_DEFINE(GetThreadContext);
	IMPORT_DEFINE(SetThreadContext);
	IMPORT_DEFINE(ResumeThread);
	IMPORT_DEFINE(GetCurrentThread);

	IMPORT_INIT
		NET_FILEMANAGER fmanager(CSTRING("C:\\Windows\\System32\\kernel32.dll"), NET_FILE_READ);
	if (!fmanager.file_exists())
		return false;

	BYTE* module = nullptr;
	size_t size = NULL;
	if (!fmanager.read(module, size))
		return false;

	handle.Set(new HMEMORYMODULE(MemoryLoadLibrary(module, size)));
	if (!handle.valid())
	{
		LOG_ERROR(CSTRING("[%s] - LoadLibrary failed with error: %ld"), MODULE_NAME, GetLastError());
		return false;
	}

	FREE(module);

	IMPORT_MPROCADDR(FindResourceA);
	IMPORT_MPROCADDR(FindResourceW);
	IMPORT_MPROCADDR(LoadResource);
	IMPORT_MPROCADDR(SizeofResource);
	IMPORT_MPROCADDR(LockResource);
	IMPORT_MPROCADDR(LoadLibraryA);
	IMPORT_MPROCADDR(LoadLibraryW);
	IMPORT_MPROCADDR(GetProcAddress);
	IMPORT_MPROCADDR(FreeLibrary);
	IMPORT_MPROCADDR(Sleep);
	IMPORT_MPROCADDR(K32EnumProcesses);
	IMPORT_MPROCADDR(OpenProcess);
	IMPORT_MPROCADDR(K32EnumProcessModules);
	IMPORT_MPROCADDR(K32GetModuleBaseNameA);
	IMPORT_MPROCADDR(CheckRemoteDebuggerPresent);
	IMPORT_MPROCADDR(IsDebuggerPresent);
	IMPORT_MPROCADDR(GetVersionExA);
	IMPORT_MPROCADDR(GetModuleHandleA);
	IMPORT_MPROCADDR(GetModuleFileNameA);
	IMPORT_MPROCADDR(MapViewOfFile);
	IMPORT_MPROCADDR(CreateFileMappingA);
	IMPORT_MPROCADDR(CreateFileA);
	IMPORT_MPROCADDR(UnmapViewOfFile);
	IMPORT_MPROCADDR(CloseHandle);
	IMPORT_MPROCADDR(GetThreadContext);
	IMPORT_MPROCADDR(SetThreadContext);
	IMPORT_MPROCADDR(ResumeThread);
	IMPORT_MPROCADDR(GetCurrentThread);

	return true;
	IMPORT_END;

	IMPORT_UNLOAD
		if (!handle.valid())
			return;

	DELETE_IMPORT(FindResourceA);
	DELETE_IMPORT(FindResourceW);
	DELETE_IMPORT(LoadResource);
	DELETE_IMPORT(SizeofResource);
	DELETE_IMPORT(LockResource);
	DELETE_IMPORT(LoadLibraryA);
	DELETE_IMPORT(LoadLibraryW);
	DELETE_IMPORT(GetProcAddress);
	DELETE_IMPORT(FreeLibrary);
	DELETE_IMPORT(Sleep);
	DELETE_IMPORT(K32EnumProcesses);
	DELETE_IMPORT(OpenProcess);
	DELETE_IMPORT(K32EnumProcessModules);
	DELETE_IMPORT(K32GetModuleBaseNameA);
	DELETE_IMPORT(CheckRemoteDebuggerPresent);
	DELETE_IMPORT(IsDebuggerPresent);
	DELETE_IMPORT(GetVersionExA);
	DELETE_IMPORT(GetModuleHandleA);
	DELETE_IMPORT(GetModuleFileNameA);
	DELETE_IMPORT(MapViewOfFile);
	DELETE_IMPORT(CreateFileMappingA);
	DELETE_IMPORT(CreateFileA);
	DELETE_IMPORT(UnmapViewOfFile);
	DELETE_IMPORT(CloseHandle);
	DELETE_IMPORT(GetThreadContext);
	DELETE_IMPORT(SetThreadContext);
	DELETE_IMPORT(ResumeThread);
	DELETE_IMPORT(GetCurrentThread);

	MemoryFreeLibrary(*handle.get());
	DELETE_IMPORT_HANDLE(handle);
	IMPORT_END;

	MAKE_IMPORT(HRSRC, FindResourceA, const HMODULE handle, const LPCSTR lpName, const LPCSTR lpType)
		PASS_PARAMETERS(handle, lpName, lpType);

	MAKE_IMPORT(HRSRC, FindResourceW, const HMODULE handle, const wchar_t* lpName, const wchar_t* lpType)
		PASS_PARAMETERS(handle, lpName, lpType);

	MAKE_IMPORT(HGLOBAL, LoadResource, const HMODULE handle, HRSRC const hResInfo)
		PASS_PARAMETERS(handle, hResInfo);

	MAKE_IMPORT(DWORD, SizeofResource, const HMODULE handle, HRSRC const hResInfo)
		PASS_PARAMETERS(handle, hResInfo);

	MAKE_IMPORT(LPVOID, LockResource, const HGLOBAL hResData)
		PASS_PARAMETERS(hResData);

	MAKE_IMPORT(HMODULE, LoadLibraryA, const LPCSTR lpLibFileName)
		PASS_PARAMETERS(lpLibFileName);

	MAKE_IMPORT(HMODULE, LoadLibraryW, const wchar_t* lpLibFileName)
		PASS_PARAMETERS(lpLibFileName);

	MAKE_IMPORT(FARPROC, GetProcAddress, const HMODULE hModule, const LPCSTR lpProcName)
		PASS_PARAMETERS(hModule, lpProcName);

	MAKE_IMPORT(BOOL, FreeLibrary, const HMODULE hModule)
		PASS_PARAMETERS(hModule);

	MAKE_IMPORT(void, Sleep, const DWORD dwMilliseconds)
		PASS_PARAMETERS(dwMilliseconds);

	MAKE_IMPORT(BOOL, K32EnumProcesses, DWORD* lpidProcess, DWORD cb, LPDWORD lpcbNeeded)
		PASS_PARAMETERS(lpidProcess, cb, lpcbNeeded);

	MAKE_IMPORT(HANDLE, OpenProcess, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
		PASS_PARAMETERS(dwDesiredAccess, bInheritHandle, dwProcessId);

	MAKE_IMPORT(BOOL, K32EnumProcessModules, HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded)
		PASS_PARAMETERS(hProcess, lphModule, cb, lpcbNeeded);

	MAKE_IMPORT(DWORD, K32GetModuleBaseNameA, HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize)
		PASS_PARAMETERS(hProcess, hModule, lpBaseName, nSize);

	MAKE_IMPORT(BOOL, CheckRemoteDebuggerPresent, HANDLE hProcess, PBOOL pbDebuggerPresent)
		PASS_PARAMETERS(hProcess, pbDebuggerPresent);

	MAKE_IMPORT(BOOL, IsDebuggerPresent)
		NO_PARAMETERS;

	MAKE_IMPORT(BOOL, GetVersionExA, LPOSVERSIONINFOA lpVersionInformation)
		PASS_PARAMETERS(lpVersionInformation);

	MAKE_IMPORT(HMODULE, GetModuleHandleA, LPCSTR lpModuleName)
		PASS_PARAMETERS(lpModuleName);

	MAKE_IMPORT(DWORD, GetModuleFileNameA, HMODULE hModule, LPSTR lpFilename, DWORD nSize)
		PASS_PARAMETERS(hModule, lpFilename, nSize);

	MAKE_IMPORT(LPVOID, MapViewOfFile, HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
		PASS_PARAMETERS(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);

	MAKE_IMPORT(HANDLE, CreateFileMappingA, HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName)
		PASS_PARAMETERS(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

	MAKE_IMPORT(HANDLE, CreateFileA, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
		PASS_PARAMETERS(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	MAKE_IMPORT(BOOL, UnmapViewOfFile, LPCVOID lpBaseAddress)
		PASS_PARAMETERS(lpBaseAddress);

	MAKE_IMPORT(BOOL, CloseHandle, HANDLE hObject)
		PASS_PARAMETERS(hObject);

	MAKE_IMPORT(BOOL, GetThreadContext, HANDLE hThread, LPCONTEXT lpContext)
		PASS_PARAMETERS(hThread, lpContext);

	MAKE_IMPORT(BOOL, SetThreadContext, HANDLE hThread, const CONTEXT* lpContext)
		PASS_PARAMETERS(hThread, lpContext);

	MAKE_IMPORT(DWORD, ResumeThread, HANDLE hThread)
		PASS_PARAMETERS(hThread);

	MAKE_IMPORT(HANDLE, GetCurrentThread)
		NO_PARAMETERS;
	IMPORT_END
}