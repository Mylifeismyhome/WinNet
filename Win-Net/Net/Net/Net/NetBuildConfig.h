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