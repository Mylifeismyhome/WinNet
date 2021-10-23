#pragma once
#define X_RESOLVER_TO_STRING(str) RESOLVER_TO_STRING(str)
#define RESOLVER_TO_STRING(str) #str

#define RESOLVE_IMPORT_BEGIN \
namespace IMPORT_NAME## { \
void OnImportLoaded(int index); \
int GetImportIndex();
#define RESOLVE_IMPORT_END }

#define DEFINE_IMPORT(Ret, funcName, ...) \
typedef Ret##(*_##funcName)(__VA_ARGS__); \
static Ret funcName##(__VA_ARGS__) \
{ \
	auto func = (_##funcName)Import::Resolver::Function(GetImportIndex(), #funcName).get(); \
	if (!func) \
	{ \
		LOG_ERROR(CSTRING("Failure on resolving %s -> %s"), CSTRING(X_RESOLVER_TO_STRING(IMPORT_NAME)), #funcName); \
	}

#define MAKE_IMPORT(...) \
	return func(__VA_ARGS__); \
}

#define IMPORT_LOAD(Module, ...) Import::Resolver::Load(#Module, Module##::OnImportLoaded, __VA_ARGS__)
#define IMPORT_UNLOAD Import::Resolver::Unload

#define RESOLVE_IMPORT_INDEX(Module) static int ___##Module##__Index = -1;
#define RESOLVE_IMPORT_INTERFACE(Module) \
RESOLVE_IMPORT_INDEX(Module) \
namespace Module \
{ \
	void OnImportLoaded(int index) \
	{ \
		___##Module##__Index = index; \
	} \
	int GetImportIndex() \
	{ \
		return ___##Module##__Index; \
	} \
}

#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/Cryption/PointerCryption.h>

#include <Net/assets/manager/logmanager.h>
#include <Net/assets/manager/filemanager.h>
#include "MemoryModule.h"

namespace Import
{
	namespace Resolver
	{
		enum class type_t
		{
			RESOLVE_KERNEL32 = 0,
			RESOLVE_MEMORY
		};

		struct function_t
		{
			char name[MAX_PATH];
			FARPROC ptr;
		};

		struct module_t
		{
			char name[MAX_PATH];
			char path[MAX_PATH];
			CPOINTER<void> module;
			type_t type;

			std::map<std::string, function_t> functionTable;
		};

		bool isLoaded(const char* library);
		module_t getModule(const char* library);
		module_t getModule(int index);
		bool Load(const char* library, void(*OnImportLoaded)(int), const char* path, type_t type = type_t::RESOLVE_KERNEL32);
		bool Remove(const char* library);
		bool Unload(const char* library);
		CPOINTER<void> Function(const char* library, const char* funcName);
		CPOINTER<void> Function(int index, const char* funcName);
	}
}