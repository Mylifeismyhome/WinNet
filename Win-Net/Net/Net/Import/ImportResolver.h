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
		LOG_ERROR(CSTRING("Failure on resolving %s -> %s [%d]"), CSTRING(X_RESOLVER_TO_STRING(IMPORT_NAME)), #funcName, GetLastError()); \
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
			CPOINTER<void> module;
			type_t type;
		};

		bool isLoaded(const char* library);
		module_t getModule(const char* library);
		bool Load(const char* library, const char* path, type_t type = type_t::RESOLVE_KERNEL32);
		bool Remove(const char* library);
		bool Unload(const char* library);
		CPOINTER<void> Function(const char* library, const char* funcName);
	}
}
#endif
