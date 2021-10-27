#pragma once
#include "NetBuildConfig.h"

// disable windows compitable code
#ifdef BUILD_LINUX
#define NET_DISABLE_MEMORY_IMPORT_DLL
#define NET_DISABLE_IMPORT_KERNEL32
#define NET_DISABLE_IMPORT_NTDLL
#define NET_DISABLE_IMPORT_WS2_32
#define NET_DISABLE_IMPORT_User32
#endif

/* disable dynamic link */
#ifdef NET_USE_MYSQL_SUPPORT
#define CPPCONN_LIB_BUILD
#endif

/* disable/enable logging */
#ifdef NET_LIB_CORE
#ifdef NET_CORE__DISABLE_LOGMANAGER
#define NET_DISABLE_LOGMANAGER
#endif
#endif

#ifdef NET_LIB_CLIENT
#ifdef NET_CLIENT__DISABLE_LOGMANAGER
#define NET_DISABLE_LOGMANAGER
#endif
#endif

#ifdef NET_LIB_SERVER
#ifdef NET_SERVER__DISABLE_LOGMANAGER
#define NET_DISABLE_LOGMANAGER
#endif
#endif

#ifdef NET_LIB_WEBSERVER
#ifdef NET_WEBSERVER__DISABLE_LOGMANAGER
#define NET_DISABLE_LOGMANAGER
#endif
#endif