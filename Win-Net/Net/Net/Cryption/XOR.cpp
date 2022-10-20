#include "XOR.h"

namespace Net
{
	namespace Cryption
	{
		XOR_UNIQUEPOINTER::XOR_UNIQUEPOINTER(char* buffer, const size_t size, const bool bFree)
		{
			// create a copy
			this->buffer = buffer; // pointer swap
			this->_size = size;
			this->bFree = bFree;
		}

		XOR_UNIQUEPOINTER::~XOR_UNIQUEPOINTER()
		{
			if (this->bFree)
				this->buffer.free();

			this->_size = NULL;
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
			return _size - 1;
		}

		size_t XOR_UNIQUEPOINTER::size() const
		{
			return _size;
		}

		void XOR_UNIQUEPOINTER::free()
		{
			buffer.free();
		}

		void XOR_UNIQUEPOINTER::lost_reference()
		{
			/*
			* lost reference means that we know that the pointer is not valid any longer
			*/
			this->bFree = false;
		}

		XOR::XOR()
		{
			_size = INVALID_SIZE;
			_buffer = nullptr;
			_Key = 0;
		}

		XOR::XOR(char* str)
		{
			init(str);
		}

		XOR::XOR(const char* str)
		{
			init(str);
		}

		void XOR::set(size_t it, char c)
		{
			if (it > size())
				return;

			this->_buffer.get()[it] = c;

			/*
			* encrypt it
			*/
			this->_buffer.get()[it] ^= (_Key % (it == 0 ? 1 : it));
		}

		void XOR::set_size(size_t new_size)
		{
			this->_size = new_size;
		}

		void XOR::init(char* str)
		{
			if (!str)
			{
				_size = INVALID_SIZE;
				_buffer = nullptr;
				_Key = 0;
				return;
			}

			_size = std::strlen(str);
			_buffer = str;
			_Key = 0;

			encrypt();
		}

		void XOR::init(const char* str)
		{
			_size = std::strlen(str);
			_buffer = ALLOC<char>(_size + 1);
			memcpy(_buffer.get(), str, _size);
			_buffer.get()[_size] = '\0';
			_Key = 0;

			encrypt();
		}

		char* XOR::encrypt()
		{
			if (size() == INVALID_SIZE)
			{
				return (char*)CSTRING("[ERROR] - Invalid size");
			}

			// gen new key
			_Key = rand();

			for (size_t i = 0; i < size(); i++)
			{
				_buffer.get()[i] = static_cast<char>(_buffer.get()[i] ^ (_Key % (i == 0 ? 1 : i)));
			}

			return _buffer.get();
		}

		XOR_UNIQUEPOINTER XOR::revert(const bool free)
		{
			NET_CPOINTER<byte> buffer(ALLOC<byte>(this->size() + 1));
			for (size_t i = 0; i < this->size(); ++i)
			{
				buffer.get()[i] = this->operator[](i);
			}
			buffer.get()[this->size()] = '\0';
			return XOR_UNIQUEPOINTER(reinterpret_cast<char*>(buffer.get()), size(), free);
		}

		size_t XOR::size() const
		{
			return _size;
		}

		size_t XOR::length() const
		{
			return _size - 1;
		}

		void XOR::free()
		{
			_Key = 0;
			_buffer.free();
			_size = INVALID_SIZE;
		}
	}
}