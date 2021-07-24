#pragma once

#undef NET_USE_MYSQL_SUPPORT
#define BUILD_LINUX

// disable windows compitable code
#ifdef BUILD_LINUX
#define NET_DISABLE_MEMORY_IMPORT_DLL
#define NET_DISABLE_IMPORT_KERNEL32
#define NET_DISABLE_IMPORT_NTDLL
#define NET_DISABLE_IMPORT_WS2_32
#endif
