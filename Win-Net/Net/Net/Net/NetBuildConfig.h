#pragma once

// need to implement
/*
mysqlcppconn.lib
mysqlcppconn-static.lib
*/

#define NET_USE_MYSQL_SUPPORT
#define BUILD_LINUX

/*
	define this to build for visual studio 2013
*/
#undef VS13

#undef NET_TEST_MEMORY_LEAKS

// disable windows compitable code
#ifdef BUILD_LINUX
#define NET_DISABLE_MEMORY_IMPORT_DLL
#define NET_DISABLE_IMPORT_KERNEL32
#define NET_DISABLE_IMPORT_NTDLL
#define NET_DISABLE_IMPORT_WS2_32
#endif
