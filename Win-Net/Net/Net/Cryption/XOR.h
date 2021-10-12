#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/PointerCryption.h>

NET_DSA_BEGIN

#define RUNTIMEXOR Net::Cryption::XOR
#ifndef VS13
#define COMPILETIME_XOR(string) [] \
{ \
	return Net::Cryption::CXOR<char, sizeof(string)/sizeof(char)>(string); \
}().get()
#define WCOMPILETIME_XOR(string) [] \
{ \
	return Net::Cryption::CXOR<wchar_t, sizeof(string)/sizeof(wchar_t)>(string); \
}().get()
#define CASTRING(string) COMPILETIME_XOR(string)
#define CWSTRING(string) WCOMPILETIME_XOR(L##string)
#endif

#ifdef VS13
#define CSTRING(string) string
#define CWSTRING(string) L##string
#define CASTRING(string) string
#else
#ifdef UNICODE
#define CSTRING(string) CWSTRING(string)
#else
#define CSTRING(string) CASTRING(string)
#endif
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NET_CLASS_BEGIN(XOR_UNIQUEPOINTER)
CPOINTER<char> buffer;
size_t _length;
bool bFree;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(XOR_UNIQUEPOINTER, char*, size_t, bool)
NET_CLASS_DESTRUCTUR(XOR_UNIQUEPOINTER)

char* get() const;
char* data()  const;
char* str() const;
size_t length() const;
size_t size() const;

void free();
NET_CLASS_END

NET_CLASS_BEGIN(XOR)
CPOINTER<char> _buffer;
size_t _length;
CPOINTER<size_t> _Key;

char* encrypt();
char* decrypt() const;
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(XOR)
NET_CLASS_CONSTRUCTUR(XOR, char*)
void init(char*);
void init(const char*);
size_t size() const;
size_t length() const;
XOR_UNIQUEPOINTER revert(bool = true);
void free();
NET_CLASS_END

#ifndef VS13
CONSTEXPR size_t COMPILETIME_XOR_LCG(const unsigned rounds) {
	return SIZE_MAX + SIZE_MAX * (rounds > 0 ? COMPILETIME_XOR_LCG(rounds - 1) : SIZE_MAX) % 0xFFFFFFFF;
}
#endif

template<typename T, const size_t len>
class CXOR
{
	T _string[len];
	size_t _key[len];

public:
	explicit CXOR(const T* string) : _string{}, _key{}
	{
		memset(_string, NULL, len);
		memset(_key, NULL, len);

		for (size_t i = 0; i < len; i++)
			_key[i] = COMPILETIME_XOR_LCG(10) % (0xFF - 0 + 1);

		for (size_t i = 0; i < len; i++)
			_string[i] = string[i] ^ _key[i];
	}

	~CXOR()
	{
		memset(_string, NULL, len);
		memset(_key, NULL, len);
	}

	CONSTEXPR T* get()
	{
		for (size_t i = 0; i < len; i++)
			_string[i] = _string[i] ^ _key[i];

		return _string;
	}
};
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
