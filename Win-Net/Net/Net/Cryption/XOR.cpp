#include "XOR.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
XOR_UNIQUEPOINTER::XOR_UNIQUEPOINTER(char* buffer, const size_t size, const bool bFree)
{
	// create a copy
	this->buffer = buffer; // pointer swap
	this->_length = size;
	this->bFree = bFree;
}

XOR_UNIQUEPOINTER::~XOR_UNIQUEPOINTER()
{
	if(this->bFree)
		this->buffer.free();

	this->_length = NULL;
}

char* XOR_UNIQUEPOINTER::get() const
{
	return buffer.get();
}

char* XOR_UNIQUEPOINTER::data() const
{
	return buffer.get();
}

char* XOR_UNIQUEPOINTER::str() const
{
	return buffer.get();
}

size_t XOR_UNIQUEPOINTER::length() const
{
	return _length;
}

size_t XOR_UNIQUEPOINTER::size() const
{
	return _length;
}

void XOR_UNIQUEPOINTER::free()
{
	buffer.free();
}

XOR::XOR()
{
	_length = -1;
	_buffer = nullptr;
	_Key = nullptr;
}

XOR::XOR(char* str)
{
	init(str);
}

void XOR::init(char* str)
{
	if (!str)
	{
		_length = -1;
		_buffer = nullptr;
		_Key = nullptr;
		return;
	}

	_length = std::strlen(str);
	_buffer = ALLOC<char>(length() + 1);
	memcpy(_buffer.get(), str, length());
	_buffer.get()[length()] = '\0';
	_Key = nullptr;

	encrypt();
}

char* XOR::encrypt()
{
	if (length() == INVALID_SIZE)
		return (char*)CSTRING("[ERROR] - Invalid length");

	_Key.free();

	_Key = ALLOC<size_t>(length());
	for (size_t i = 0; i < length(); i++)
		_Key.get()[i] = rand();

	if (!_Key.valid())
		return (char*)CSTRING("[ERROR] - Invalid Key");

	for (size_t i = 0; i < length(); i++)
		_buffer.get()[i] = static_cast<char>(_buffer.get()[i] ^ _Key.get()[i]);

	return _buffer.get();
}

char* XOR::decrypt() const
{
	if (length() == INVALID_SIZE)
		return (char*)CSTRING("[ERROR] - Invalid length");

	if (!_Key.valid())
		return (char*)CSTRING("[ERROR] - Invalid Key");

	for (size_t i = 0; i < length(); i++)
		_buffer.get()[i] = static_cast<char>(_buffer.get()[i] ^ _Key.get()[i]);

	return _buffer.get();
}

XOR_UNIQUEPOINTER XOR::revert(const bool free)
{
	const auto pointer = decrypt();

	// create a copy
	const auto decrypted = ALLOC<char>(length() + 1);
	memcpy(decrypted, pointer, length());
	decrypted[length()] = '\0';

	encrypt();

	return XOR_UNIQUEPOINTER(decrypted, length(), free);
}

size_t XOR::size() const
{
	return _length;
}

size_t XOR::length() const
{
	return _length;
}

void XOR::free()
{
	_Key.free();
	_buffer.free();
}
NET_NAMESPACE_END
NET_NAMESPACE_END
