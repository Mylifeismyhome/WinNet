#pragma once
#ifndef BUILD_LINUX
#include "ImportResolver.h"

#ifdef NET_DISABLE_IMPORT_KERNEL32
#define Kernel32
#else
#include <TlHelp32.h>

#define IMPORT_NAME Kernel32

RESOLVE_IMPORT_BEGIN;
DEFINE_IMPORT(HRSRC, FindResourceA, HMODULE handle, LPCSTR lpName, LPCSTR lpType);
MAKE_IMPORT(handle, lpName, lpType);

DEFINE_IMPORT(HRSRC, FindResourceW, HMODULE handle, wchar_t* lpName, wchar_t* lpType);
MAKE_IMPORT(handle, lpName, lpType);

DEFINE_IMPORT(HGLOBAL, LoadResource, HMODULE handle, HRSRC hResInfo);
MAKE_IMPORT(handle, hResInfo);

DEFINE_IMPORT(DWORD, SizeofResource, HMODULE handle, HRSRC hResInfo);
MAKE_IMPORT(handle, hResInfo);

DEFINE_IMPORT(LPVOID, LockResource, HGLOBAL hResData);
MAKE_IMPORT(hResData);

DEFINE_IMPORT(HMODULE, LoadLibraryA, LPCSTR lpLibFileName);
MAKE_IMPORT(lpLibFileName);

DEFINE_IMPORT(HMODULE, LoadLibraryW, const wchar_t* lpLibFileName);
MAKE_IMPORT(lpLibFileName);

DEFINE_IMPORT(FARPROC, GetProcAddress, HMODULE hModule, LPCSTR lpProcName);
MAKE_IMPORT(hModule, lpProcName);

DEFINE_IMPORT(BOOL, FreeLibrary, HMODULE hModule);
MAKE_IMPORT(hModule);

DEFINE_IMPORT(void, Sleep, DWORD dwMilliseconds);
MAKE_IMPORT(dwMilliseconds);

DEFINE_IMPORT(BOOL, K32EnumProcesses, DWORD* lpidProcess, DWORD cb, LPDWORD lpcbNeeded);
MAKE_IMPORT(lpidProcess, cb, lpcbNeeded);

DEFINE_IMPORT(HANDLE, OpenProcess, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
MAKE_IMPORT(dwDesiredAccess, bInheritHandle, dwProcessId);

DEFINE_IMPORT(BOOL, K32EnumProcessModules, HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
MAKE_IMPORT(hProcess, lphModule, cb, lpcbNeeded);

DEFINE_IMPORT(DWORD, K32GetModuleBaseNameA, HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
MAKE_IMPORT(hProcess, hModule, lpBaseName, nSize);

DEFINE_IMPORT(BOOL, CheckRemoteDebuggerPresent, HANDLE hProcess, PBOOL pbDebuggerPresent);
MAKE_IMPORT(hProcess, pbDebuggerPresent);

DEFINE_IMPORT(BOOL, IsDebuggerPresent);
MAKE_IMPORT();

DEFINE_IMPORT(BOOL, GetVersionExA, LPOSVERSIONINFOA lpVersionInformation);
MAKE_IMPORT(lpVersionInformation);

DEFINE_IMPORT(HMODULE, GetModuleHandleA, LPCSTR lpModuleName);
MAKE_IMPORT(lpModuleName);

DEFINE_IMPORT(DWORD, GetModuleFileNameA, HMODULE hModule, LPSTR lpFilename, DWORD nSize);
MAKE_IMPORT(hModule, lpFilename, nSize);

DEFINE_IMPORT(LPVOID, MapViewOfFile, HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
MAKE_IMPORT(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);

DEFINE_IMPORT(HANDLE, CreateFileMappingA, HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
MAKE_IMPORT(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

DEFINE_IMPORT(HANDLE, CreateFileA, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
MAKE_IMPORT(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

DEFINE_IMPORT(BOOL, UnmapViewOfFile, LPCVOID lpBaseAddress);
MAKE_IMPORT(lpBaseAddress);

DEFINE_IMPORT(BOOL, CloseHandle, HANDLE hObject);
MAKE_IMPORT(hObject);

DEFINE_IMPORT(BOOL, GetThreadContext, HANDLE hThread, LPCONTEXT lpContext);
MAKE_IMPORT(hThread, lpContext);

DEFINE_IMPORT(BOOL, SetThreadContext, HANDLE hThread, CONTEXT* lpContext);
MAKE_IMPORT(hThread, lpContext);

DEFINE_IMPORT(DWORD, ResumeThread, HANDLE hThread);
MAKE_IMPORT(hThread);

DEFINE_IMPORT(HANDLE, GetCurrentThread);
MAKE_IMPORT();

DEFINE_IMPORT(PVOID, AddVectoredExceptionHandler, ULONG First, PVECTORED_EXCEPTION_HANDLER Handler);
MAKE_IMPORT(First, Handler);

DEFINE_IMPORT(VOID, RaiseException, DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD  nNumberOfArguments, ULONG_PTR* lpArguments);
MAKE_IMPORT(dwExceptionCode, dwExceptionFlags, nNumberOfArguments, lpArguments);

DEFINE_IMPORT(LPVOID, VirtualAlloc, LPVOID lpAddress, SIZE_T dwSize, DWORD  flAllocationType, DWORD  flProtect);
MAKE_IMPORT(lpAddress, dwSize, flAllocationType, flProtect);

DEFINE_IMPORT(BOOL, VirtualFree, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
MAKE_IMPORT(lpAddress, dwSize, dwFreeType);

DEFINE_IMPORT(void, OutputDebugStringA, PCSTR lpOutputString);
MAKE_IMPORT(lpOutputString);

DEFINE_IMPORT(DWORD, GetLastError);
MAKE_IMPORT();

DEFINE_IMPORT(DWORD, GetTickCount);
MAKE_IMPORT();

DEFINE_IMPORT(void, GetSystemTime, LPSYSTEMTIME lpSystemTime);
MAKE_IMPORT(lpSystemTime);

DEFINE_IMPORT(void, GetLocalTime, LPSYSTEMTIME lpSystemTime);
MAKE_IMPORT(lpSystemTime);

DEFINE_IMPORT(BOOL, SystemTimeToFileTime, SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime);
MAKE_IMPORT(lpSystemTime, lpFileTime);

DEFINE_IMPORT(BOOL, QueryPerformanceCounter, LARGE_INTEGER* lpPerformanceCount);
MAKE_IMPORT(lpPerformanceCount);

DEFINE_IMPORT(BOOL, VirtualProtect, LPVOID lpAddress, SIZE_T dwSize, DWORD  flNewProtect, PDWORD lpflOldProtect);
MAKE_IMPORT(lpAddress, dwSize, flNewProtect, lpflOldProtect);

DEFINE_IMPORT(BOOL, ReadFile, HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
MAKE_IMPORT(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);

DEFINE_IMPORT(BOOL, WriteProcessMemory, HANDLE  hProcess, LPVOID  lpBaseAddress, LPCVOID lpBuffer, SIZE_T  nSize, SIZE_T* lpNumberOfBytesWritten);
MAKE_IMPORT(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);

DEFINE_IMPORT(HANDLE, GetCurrentProcess);
MAKE_IMPORT();

DEFINE_IMPORT(BOOL, Toolhelp32ReadProcessMemory, DWORD th32ProcessID, LPCVOID lpBaseAddress, LPVOID  lpBuffer, SIZE_T  cbRead, SIZE_T* lpNumberOfBytesRead);
MAKE_IMPORT(th32ProcessID, lpBaseAddress, lpBuffer, cbRead, lpNumberOfBytesRead);

DEFINE_IMPORT(DWORD, GetCurrentProcessId)
MAKE_IMPORT();

DEFINE_IMPORT(HANDLE, CreateToolhelp32Snapshot, DWORD dwFlags, DWORD th32ProcessID);
MAKE_IMPORT(dwFlags, th32ProcessID);

DEFINE_IMPORT(BOOL, Process32FirstW, HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
MAKE_IMPORT(hSnapshot, lppe);

DEFINE_IMPORT(BOOL, ReadProcessMemory, HANDLE  hProcess, LPCVOID lpBaseAddress, LPVOID  lpBuffer, SIZE_T  nSize, SIZE_T* lpNumberOfBytesRead);
MAKE_IMPORT(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);

DEFINE_IMPORT(BOOL, Process32NextW, HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
MAKE_IMPORT(hSnapshot, lppe);

DEFINE_IMPORT(BOOL, TerminateProcess, HANDLE hProcess, UINT uExitCode);
MAKE_IMPORT(hProcess, uExitCode);

DEFINE_IMPORT(BOOL, Process32First, HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
MAKE_IMPORT(hSnapshot, lppe);

DEFINE_IMPORT(BOOL, Process32Next, HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
MAKE_IMPORT(hSnapshot, lppe);

DEFINE_IMPORT(HANDLE, OpenThread, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
MAKE_IMPORT(dwDesiredAccess, bInheritHandle, dwThreadId);

DEFINE_IMPORT(DWORD, SuspendThread, HANDLE hThread);
MAKE_IMPORT(hThread);

DEFINE_IMPORT(BOOL, Thread32Next, HANDLE hSnapshot, LPTHREADENTRY32 lpte);
MAKE_IMPORT(hSnapshot, lpte);

DEFINE_IMPORT(DWORD, GetFileSize, HANDLE hFile, LPDWORD lpFileSizeHigh);
MAKE_IMPORT(hFile, lpFileSizeHigh);
RESOLVE_IMPORT_END;
#endif
#endif
