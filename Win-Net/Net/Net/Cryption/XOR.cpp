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

#include "XOR.h"

namespace Net
{
	namespace Cryption
	{
		XOR_UNIQUEPOINTER::XOR_UNIQUEPOINTER()
		{
			m_refCount = 1;
			m_buffer = nullptr;
			m_size = INVALID_SIZE;
		}

		XOR_UNIQUEPOINTER::XOR_UNIQUEPOINTER(char* buffer, const size_t size, size_t refCount)
		{
			m_refCount = refCount;
			m_buffer = buffer;
			m_size = size;
		}

		XOR_UNIQUEPOINTER::~XOR_UNIQUEPOINTER()
		{
			if(m_refCount != 0)
			{
				m_refCount--;
			}

			if (m_refCount <= 0)
			{
				m_buffer.free();
			}
		}

		XOR_UNIQUEPOINTER& XOR_UNIQUEPOINTER::operator=(const XOR_UNIQUEPOINTER& other)
		{
			// Guard self assignment
			if (this == &other)
			{
				return *this;
			}

			m_buffer = other.m_buffer;
			m_size = other.m_size;
			m_refCount = other.m_refCount;
			const_cast<XOR_UNIQUEPOINTER*>(&other)->m_refCount++;

			return *this;
		}

		char* XOR_UNIQUEPOINTER::get() const
		{
			return m_buffer.get();
		}

		char* XOR_UNIQUEPOINTER::data() const
		{
			return m_buffer.get();
		}

		char* XOR_UNIQUEPOINTER::str() const
		{
			return m_buffer.get();
		}

		size_t XOR_UNIQUEPOINTER::length() const
		{
			return m_size - 1;
		}

		size_t XOR_UNIQUEPOINTER::size() const
		{
			return m_size;
		}

		void XOR_UNIQUEPOINTER::free()
		{
			m_buffer.free();
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
			_buffer.free();

			if (m_size >= size())
			{
				auto tmp = ALLOC<char>(m_size + 1);
				for (size_t i = 0; i < size(); ++i)
				{
					tmp[i] = this->operator[](i);
				}
				tmp[size()] = 0;

				_buffer = tmp;
				_actual_size = m_size;
				return;
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
			{
				return *this;
			}

			_buffer = other._buffer;
			_Key = other._Key;
			_size = other._size;
			_actual_size = other._actual_size;
			const_cast<XOR*>(&other)->_buffer = nullptr;

			return *this;
		}

		char XOR::operator[](size_t i)
		{
			auto buffer_ptr = this->_buffer.get();
			if (!buffer_ptr) return 0;
			return static_cast<char>(buffer_ptr[i] ^ (this->_Key % (i == 0 ? 1 : i)));
		}

		void XOR::set(size_t it, char c)
		{
			if (it > actual_size() || actual_size() == INVALID_SIZE)
			{
				return;
			}

			this->_buffer.get()[it] = c;

			/*
			* encrypt it
			*/
			this->_buffer.get()[it] ^= (this->_Key % (it == 0 ? 1 : it));
		}

		void XOR::set_size(size_t new_size)
		{
			if (this->_actual_size < new_size)
			{
				/*
				* reached limit
				* require realloc
				*/
				this->reserve(new_size);
			}

			this->_size = new_size;
		}

		void XOR::init(char* str)
		{
			if (str == nullptr)
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

		char* XOR::decrypt()
		{
			if (size() == INVALID_SIZE)
			{
				return nullptr;
			}

			for (size_t i = 0; i < size(); i++)
			{
				_buffer.get()[i] = static_cast<char>(_buffer.get()[i] ^ (_Key % (i == 0 ? 1 : i)));
			}

			return _buffer.get();
		}

		XOR_UNIQUEPOINTER XOR::revert()
		{
			const auto len = size();
			auto ptr = ALLOC<char>(len + 1);
			if (ptr == nullptr)
			{
				return {};
			}

			for (size_t i = 0; i < len; ++i)
			{
				ptr[i] = operator[](i);
			}
			ptr[len] = 0;

			return { ptr, len, 1 };
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