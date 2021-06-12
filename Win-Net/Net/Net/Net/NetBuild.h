#pragma once

#include "NetBuildConfig.h"

#ifdef NET_USE_MYSQL_SUPPORT
#pragma comment(lib, "mysqlcppconn.lib")
#pragma comment(lib, "mysqlcppconn-static.lib")
#endif