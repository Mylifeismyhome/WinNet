#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>
#include "MemoryModule.h"

typedef HRSRC(WINAPI* DEF_FindResourceA)(HMODULE, LPCSTR, LPCSTR);
typedef HRSRC(WINAPI* DEF_FindResourceW)(HMODULE, const wchar_t*, const wchar_t*);
typedef HGLOBAL(WINAPI* DEF_LoadResource)(HMODULE, HRSRC);
typedef DWORD(WINAPI* DEF_SizeofResource)(HMODULE, HRSRC);
typedef LPVOID(WINAPI* DEF_LockResource)(HGLOBAL);
typedef HMODULE(WINAPI* DEF_LoadLibraryA)(LPCSTR);
typedef HMODULE(WINAPI* DEF_LoadLibraryW)(const wchar_t*);
typedef FARPROC(WINAPI* DEF_GetProcAddress)(HMODULE, LPCSTR);
typedef BOOL(WINAPI* DEF_FreeLibrary)(HMODULE);
typedef void(WINAPI* DEF_Sleep)(DWORD);

namespace Net
{
	namespace Kernel32
	{
		bool Initialize();
		void Uninitialize();

		HRSRC FindResourceA(HMODULE, LPCSTR, LPCSTR);
		HRSRC FindResourceW(HMODULE, const wchar_t*, const wchar_t*);
		HGLOBAL LoadResource(HMODULE, HRSRC);
		DWORD SizeofResource(HMODULE, HRSRC);
		LPVOID LockResource(HGLOBAL);
		HMODULE LoadLibraryA(LPCSTR);
		HMODULE LoadLibraryW(const wchar_t*);
		FARPROC GetProcAddress(HMODULE, LPCSTR);
		BOOL FreeLibrary(HMODULE);
		void Sleep(DWORD);
	}
}