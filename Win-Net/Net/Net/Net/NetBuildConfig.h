#pragma once

/* define to use MYSQL c++ connector */
#define NET_USE_MYSQL_SUPPORT

/* define to build for linux */
#undef BUILD_LINUX

/* define this to build for visual studio 2013 */
#undef VS13

/* define to track memory */
#undef NET_TEST_MEMORY_LEAKS

/* define to disable logging */
#undef NET_CORE__DISABLE_LOGMANAGER /* enable to disable logmanager for core only */
#undef NET_CLIENT__DISABLE_LOGMANAGER /* enable to disable logmanager for client only */
#undef NET_SERVER__DISABLE_LOGMANAGER /* enable to disable logmanager for server only */
#undef NET_WEBSERVER__DISABLE_LOGMANAGER /* enable to disable logmanager for webserver only */
#undef NET_DISABLE_LOGMANAGER /* enable to disable entire logmanager for every porject */

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
