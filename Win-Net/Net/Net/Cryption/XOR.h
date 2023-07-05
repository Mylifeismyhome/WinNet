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
#include <Net/Net/Net.h>
#include <Net/Cryption/PointerCryption.h>
#include <Net/Cryption/CXOR.hpp>

#define RUNTIMEXOR Net::Cryption::XOR

#ifdef BUILD_LINUX
#define CASTRING(string) string
#define CWSTRING(string) L##string
#else
#define COMPILETIME_XOR(string) xorstr_(string)
#define WCOMPILETIME_XOR(string) xorstr_(string)
#define CASTRING(string) COMPILETIME_XOR(string)
#define CWSTRING(string) WCOMPILETIME_XOR(L##string)
#endif

#ifdef NET_DISABLE_XOR_STRING_COMPILETIME
#define CSTRING(string) string
#else
#ifdef UNICODE
#define CSTRING(string) CWSTRING(string)
#else
#define CSTRING(string) CASTRING(string)
#endif
#endif

NET_DSA_BEGIN
namespace Net
{
	namespace Cryption
	{
		class XOR_UNIQUEPOINTER
		{
			BYTE bFree;
			NET_CPOINTER<char> buffer;
			size_t _size;

		public:
			XOR_UNIQUEPOINTER();
			XOR_UNIQUEPOINTER(char*, size_t, BYTE = 0);
			~XOR_UNIQUEPOINTER();

			XOR_UNIQUEPOINTER& operator=(const XOR_UNIQUEPOINTER& other);

			char* get() const;
			char* data()  const;
			char* str() const;
			size_t length() const;
			size_t size() const;

			void free();
			BYTE getFree() const;
			void setFree(BYTE);
		};

		class XOR
		{
			NET_CPOINTER<char> _buffer;
			size_t _size;
			size_t _actual_size;
			uintptr_t _Key;

			char* encrypt();
			char* decrypt();

		public:
			XOR();
			XOR(char*);
			XOR(const char*);

			void reserve(size_t size);
			void finalize();

			XOR& operator=(const XOR& other);

			/*
			* Return the character in the buffer located at index i decrypted
			*/
			char operator[](size_t i);

			void set(size_t, char);
			void set_size(size_t);
			void init(char*);
			void init(const char*);
			size_t size() const;
			size_t actual_size() const;
			size_t length() const;
			XOR_UNIQUEPOINTER revert();
			void free();
		};
	}
}
NET_DSA_END