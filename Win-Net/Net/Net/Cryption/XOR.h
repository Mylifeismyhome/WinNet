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
			XOR_UNIQUEPOINTER(char*, size_t, bool);
			~XOR_UNIQUEPOINTER();

			char* get() const;
			char* data()  const;
			char* str() const;
			size_t length() const;
			size_t size() const;

			void free();
			void lost_reference();
		};

		class XOR
		{
			NET_CPOINTER<char> _buffer;
			size_t _size;
			uintptr_t _Key;

			char* encrypt();

		public:
			XOR();
			XOR(char*);
			XOR(const char*);

			XOR& operator=(XOR& other)
			{
				// Guard self assignment
				if (this == &other)
					return *this;

				this->_buffer = other._buffer;
				this->_Key = other._Key;
				this->_size = other._size;

				other._buffer = {};
				other._Key = 0;
				other._size = 0;

				return *this;
			}

			/*
			* Return the character in the buffer located at index i decrypted
			*/
			char operator[](size_t i)
			{
				auto buffer_ptr = this->_buffer.get();
				return static_cast<char>(buffer_ptr[i] ^ (this->_Key % (i == 0 ? 1 : i)));
			}

			void init(char*);
			void init(const char*);
			size_t size() const;
			size_t length() const;
			XOR_UNIQUEPOINTER revert(bool = true);
			void free();
		};
	}
}
NET_DSA_END