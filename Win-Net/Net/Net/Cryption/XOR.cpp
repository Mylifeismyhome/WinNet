#include "XOR.h"

namespace Net
{
	namespace Cryption
	{
		XOR_UNIQUEPOINTER::XOR_UNIQUEPOINTER()
		{
			this->buffer = nullptr;
			this->_size = INVALID_SIZE;
			this->bFree = false;
		}

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

		void XOR_UNIQUEPOINTER::lost_reference() const
		{
			/*
			* lost reference means that we know that the pointer is not valid any longer
			*/
			const_cast<XOR_UNIQUEPOINTER*>(this)->bFree = false;
		}

		XOR::XOR()
		{
			_size = INVALID_SIZE;
			_actual_size = INVALID_SIZE;
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

		void XOR::reserve(size_t m_size)
		{
			if (_buffer.get())
			{
				if (m_size >= size())
				{
					auto tmp = ALLOC<char>(m_size + 1);
					for (size_t i = 0; i < size(); ++i)
					{
						tmp[i] = this->operator[](i);
					}
					tmp[size()] = 0;

					_buffer.free();
					_buffer = tmp;
					_actual_size = m_size;
					return;
				}

				_buffer.free();
			}

			auto tmp = ALLOC<char>(m_size + 1);
			memset(tmp, 0, m_size);
			_buffer = tmp;
			_size = 0;
			_actual_size = m_size;
		}

		void XOR::finalize()
		{
			if (!_buffer.get())
			{
				return;
			}

			if (actual_size() == size())
			{
				return;
			}

			auto tmp = ALLOC<char>(size() + 1);
			for (size_t i = 0; i < size(); ++i)
			{
				tmp[i] = this->operator[](i);
			}
			tmp[size()] = 0;

			_buffer.free();
			_buffer = tmp;
			_actual_size = size();
		}

		XOR& XOR::operator=(const XOR& other)
		{
			// Guard self assignment
			if (this == &other)
				return *this;

			this->_buffer = other._buffer;
			this->_Key = other._Key;
			this->_size = other._size;
			this->_actual_size = other._actual_size;

			return *this;
		}

		char XOR::operator[](size_t i)
		{
			auto buffer_ptr = this->_buffer.get();
			return static_cast<char>(buffer_ptr[i] ^ (this->_Key % (i == 0 ? 1 : i)));
		}

		void XOR::set(size_t it, char c)
		{
			if (it > actual_size() || actual_size() == INVALID_SIZE)
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
            this->_actual_size = new_size;
		}

		void XOR::init(char* str)
		{
			if (!str)
			{
				_size = INVALID_SIZE;
                _actual_size = size();
				_buffer = nullptr;
				_Key = 0;
				return;
			}

			_size = strlen(str);
            _actual_size = size();
			_buffer = str;
			_Key = 0;

			encrypt();
		}

		void XOR::init(const char* str)
		{
			_size = strlen(str);
			_actual_size = _size;
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
				return nullptr;
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

		size_t XOR::actual_size() const
		{
			return _actual_size;
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
			_actual_size = INVALID_SIZE;
		}
	}
}