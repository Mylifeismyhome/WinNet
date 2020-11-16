#pragma once
#define NET_STRING_NOT_FOUND (size_t)(~0)

#include <Net/Net/Net.h>
#include <Cryption/XOR.h>

NET_DSA_BEGIN

NET_CLASS_BEGIN(NetUniquePointer)
void* _pointer;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(NetUniquePointer, void*);
NET_CLASS_DESTRUCTUR(NetUniquePointer);

template<typename T>
T get() const
{
	return (T)_pointer;
}

template<typename T>
T pointer() const
{
	return (T)_pointer;
}

template<typename T>
T data() const
{
	return (T)_pointer;
}
NET_CLASS_END

NET_CLASS_BEGIN(NetString)
RUNTIMEXOR _string;
size_t _size;
Net::Cryption::XOR_UNIQUEPOINTER revert();

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(NetString);
NET_CLASS_CONSTRUCTUR(NetString, char, ...);
NET_CLASS_CONSTRUCTUR(NetString, const char*, ...);
NET_CLASS_CONSTRUCTUR(NetString, NetString&);
NET_CLASS_CONSTRUCTUR_NOEXCEPT(NetString, NetString&&);
NET_CLASS_DESTRUCTUR(NetString);

enum type
{
	NONE = 0x0,
	NOT_CASE_SENS,
	IN_LEN
};

void operator=(const char* in)
{
	set(in);
}

void operator=(char* in)
{
	set(in);
}

void operator=(const char in)
{
	set(in);
}

void operator=(NetString in)
{
	copy(in);
}

void operator+=(const char* in)
{
	if (size() > 0)
		append(in);
	else
		set(in);
}

void operator+=(char* in)
{
	if (size() > 0)
		append(in);
	else
		set(in);
}

void operator+=(const char in)
{
	if (size() > 0)
		append(in);
	else
		set(in);
}

void operator+=(NetString& in)
{
	if (size() > 0)
		append(in);
	else
		set(in);
}

void operator-=(const char* in)
{
	erase(in);
}

void operator-=(char* in)
{
	erase(in);
}

void operator-=(const char in)
{
	erase(in);
}

void operator-=(NetString& in)
{
	erase(in);
}

size_t size() const;
size_t length() const;
void set(char, ...);
void append(char);
void set(const char*, ...);
void append(const char*, ...);
void set(NetString&, ...);
void append(NetString&, ...);
void copy(NetString&);
void move(NetString&&);
Net::Cryption::XOR_UNIQUEPOINTER str();
Net::Cryption::XOR_UNIQUEPOINTER cstr();
Net::Cryption::XOR_UNIQUEPOINTER get();
Net::Cryption::XOR_UNIQUEPOINTER data();
void clear();
bool empty();
char at(size_t);
char* substr(size_t);
char* substr(size_t, size_t);
size_t find(char);
size_t find(char, char);
size_t find(const char*);
size_t find(const char*, char);
size_t find(size_t, char);
size_t find(size_t, char, char);
size_t find(size_t, char*);
size_t find(size_t, char*, char);
size_t find(size_t, const char*);
size_t find(size_t, const char*, char);
std::vector<size_t> findAll(char);
std::vector<size_t> findAll(char, char);
std::vector<size_t> findAll(const char*);
std::vector<size_t> findAll(const char*, char);
bool compare(char);
bool compare(char, char = NONE);
bool compare(const char*);
bool compare(const char*, char);
bool compare(char*);
bool compare(char*, char = NONE);
bool erase(size_t);
bool erase(size_t, size_t);
bool erase(char, size_t = 0);
bool erase(char, char);
bool erase(char, size_t, char);
bool erase(const char*, size_t = 0);
bool erase(const char*, char);
bool erase(const char*, size_t, char);
bool erase(NetString&, size_t = 0);
bool erase(NetString&, char);
bool erase(NetString&, size_t, char);
bool eraseAll(char);
bool eraseAll(char, char);
bool eraseAll(const char*);
bool eraseAll(const char*, char);
bool replace(char, char, size_t = 0);
bool replace(char, const char*, size_t = 0);
bool replace(const char*, char, size_t = 0);
bool replace(const char*, const char*, size_t = 0);
bool replaceAll(char, char);
bool replaceAll(char, const char*);
bool replaceAll(const char*, char);
bool replaceAll(const char*, const char*);
NET_CLASS_END

NET_DSA_END