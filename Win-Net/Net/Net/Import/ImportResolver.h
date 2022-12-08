/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once
#ifndef BUILD_LINUX
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/Cryption/PointerCryption.h>
#include <Net/assets/manager/logmanager.h>
#include <Net/assets/manager/filemanager.h>
#include "MemoryModule.h"

#ifdef NET_X64
#define WINDOWS_MODULE_PATH "C:\\Windows\\System32"
#else
#define WINDOWS_MODULE_PATH "C:\\Windows\\SysWOW64"
#endif

#define X_RESOLVER_TO_STRING(str) RESOLVER_TO_STRING(str)
#define RESOLVER_TO_STRING(str) #str

#define RESOLVE_IMPORT_BEGIN namespace IMPORT_NAME##{
#define RESOLVE_IMPORT_END }

#define DEFINE_IMPORT(Ret, funcName, ...) \
typedef Ret##(*_##funcName)(__VA_ARGS__); \
static Ret funcName##(__VA_ARGS__) \
{ \
	auto func = (_##funcName)Import::Resolver::Function(CSTRING(X_RESOLVER_TO_STRING(IMPORT_NAME)), #funcName).get(); \
	if (!func) \
	{ \
		NET_LOG_ERROR(CSTRING("Failure on resolving %s -> %s [%d]"), CSTRING(X_RESOLVER_TO_STRING(IMPORT_NAME)), #funcName, GetLastError()); \
	}

#define MAKE_IMPORT(...) \
	return func(__VA_ARGS__); \
}

namespace Import
{
	namespace Resolver
	{
		enum class type_t
		{
			RESOLVE_KERNEL32 = 0,
			RESOLVE_MEMORY,
			RESOLVE_IAT
		};

		struct module_t
		{
			char name[MAX_PATH];
			char path[MAX_PATH];
			NET_CPOINTER<void> module;
			type_t type;
		};

		bool isLoaded(const char* library);
		module_t getModule(const char* library);
		bool Load(const char* library, const char* path, type_t type = type_t::RESOLVE_KERNEL32);
		bool Remove(const char* library);
		bool Unload(const char* library);
		NET_CPOINTER<void> Function(const char* library, const char* funcName);
	}
}
#endif
