#pragma once

/* define to use MYSQL c++ connector */
#define NET_USE_MYSQL_SUPPORT

/* define to track memory */
#undef NET_TEST_MEMORY_LEAKS

/* define to disable logging */
#undef NET_CORE__DISABLE_LOGMANAGER /* enable to disable logmanager for core only */
#undef NET_CLIENT__DISABLE_LOGMANAGER /* enable to disable logmanager for client only */
#undef NET_SERVER__DISABLE_LOGMANAGER /* enable to disable logmanager for server only */
#undef NET_WEBSERVER__DISABLE_LOGMANAGER /* enable to disable logmanager for webserver only */
#undef NET_DISABLE_LOGMANAGER /* enable to disable entire logmanager for every porject */

/* define to disable XOR feature */
#undef NET_DISABLE_XOR_STRING_COMPILETIME /* enable to disable xor string at compile time */