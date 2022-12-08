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
#include "NetBuildConfig.h"

#if !defined NET_X64 && !defined NET_X86
#if defined _WIN64 || defined __x86_64__
#define NET_X64
#else
#define NET_X86
#endif
#endif

// disable windows compitable code
#ifdef BUILD_LINUX
#define NET_DISABLE_MEMORY_IMPORT_DLL
#define NET_DISABLE_IMPORT_KERNEL32
#define NET_DISABLE_IMPORT_NTDLL
#define NET_DISABLE_IMPORT_WS2_32
#define NET_DISABLE_IMPORT_User32
#endif

#ifdef DLL
#define NET_DISABLE_MEMORY_IMPORT_DLL
#define NET_DISABLE_IMPORT_KERNEL32
#define NET_DISABLE_IMPORT_NTDLL
#define NET_DISABLE_IMPORT_WS2_32
#define NET_DISABLE_IMPORT_User32
#endif

#ifdef NET_EXCLUDE_MYSQL_SUPPORT
#undef NET_USE_MYSQL_SUPPORT
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

#ifdef NET_DISABLE_IMPORT_WS2_32
#define Ws2_32
#endif