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
			NET_CPOINTER<char> buffer;
			size_t _size;
			bool bFree;

		public:
			XOR_UNIQUEPOINTER();
			XOR_UNIQUEPOINTER(char*, size_t, bool);
			~XOR_UNIQUEPOINTER();

			char* get() const;
			char* data()  const;
			char* str() const;
			size_t length() const;
			size_t size() const;

			void free();
			void lost_reference() const;
		};

		class XOR
		{
			NET_CPOINTER<char> _buffer;
			size_t _size;
			size_t _actual_size;
			uintptr_t _Key;

			char* encrypt();

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
			XOR_UNIQUEPOINTER revert(bool = true);
			void free();
		};
	}
}
NET_DSA_END