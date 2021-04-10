#pragma once
#define IMPORT_HANDLE(type, name) static CPOINTER<##type> ##name
#define IMPORT_DEFINE(name) static CPOINTER<DEF_##name> _##name
#define IMPORT_DEFINE_FNC_WINAPI(type, name, ...) typedef type(WINAPI* DEF_##name)(__VA_ARGS__); \
	type name(__VA_ARGS__);

#define IMPORT_DEFINE_FNC_WSAAPI(type, name, ...) typedef type(WSAAPI* DEF_##name)(__VA_ARGS__); \
	type name(__VA_ARGS__);

#define IMPORT_DEFINE_FNC_NTAPI(type, name, ...) typedef type(NTAPI* DEF_##name)(__VA_ARGS__); \
	type name(__VA_ARGS__);

#define IMPORT_DEFINE_FNC(type, name, ...) typedef type(* DEF_##name)(__VA_ARGS__); \
	type name(__VA_ARGS__);

#define IMPORT_MPROCADDR(name) _##name.Set(new DEF_##name((DEF_##name)MemoryGetProcAddress(*handle.get(), CSTRING(#name)))); \
	if(!_##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[%s] - Unable to resolve %s"), MODULE_NAME, CSTRING(#name)); \
		return false; \
	}

#define LIMPORT(name) _##name.Set(new DEF_##name((DEF_##name)Kernel32::GetProcAddress(*handle.get(), CSTRING(#name)))); \
	if(!_##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[%s] - Unable to resolve %s"), MODULE_NAME, CSTRING(#name)); \
		return false; \
	}

#define DLIMPORT(name) _##name.Set(new DEF_##name((DEF_##name)GetProcAddress(*handle.get(), CSTRING(#name)))); \
	if(!_##name.valid()) \
	{ \
		Uninitialize(); \
		LOG_ERROR(CSTRING("[%s] - Unable to resolve %s"), MODULE_NAME, CSTRING(#name)); \
		return false; \
	}

#define DELETE_IMPORT(pointer) delete _##pointer.get(); _##pointer = nullptr;
#define DELETE_IMPORT_HANDLE(pointer) delete pointer.get(); pointer = nullptr;

#define MAKE_IMPORT(type, name, ...) type name(__VA_ARGS__) \
{ \
	return (*_##name.get())

#define PASS_PARAMETERS(...) \
	(__VA_ARGS__); \
}

#define NO_PARAMETERS \
	(); \
}

#define IMPORT_BEGIN(name) namespace name { \
bool Initialize(); \
void Uninitialize();

#define IMPORT_END }

#define IMPORT_INIT bool Initialize() \
{

#define IMPORT_UNLOAD void Uninitialize() \
{