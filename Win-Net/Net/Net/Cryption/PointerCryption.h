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
#define NET_CPOINTER Net::Cryption::Pointer

#include <Net/Net/Net.h>

#define RAND_NUMBER rand() % INT_MAX;

NET_DSA_BEGIN
namespace Net
{
	namespace Cryption
	{
		template <typename T>
		class Cryption
		{
		protected:
			uintptr_t _key;

		public:
			T* encode(T* pointer)
			{
				pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
				return pointer;
			}

			T* encode(const T*& pointer)
			{
				pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
				return pointer;
			}

			T* decode(T* pointer) const
			{
				pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
				return pointer;
			}

			T*& decodeRef(T*& pointer)
			{
				pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
				return pointer;
			}
		};

		template <typename T>
		class UniquePointer : public Cryption<T>
		{
			T** _pointer;

		public:
			explicit UniquePointer(T** pointer, const uintptr_t key)
			{
				this->_key = key;
				_pointer = pointer;
			}

			~UniquePointer()
			{
				if (*_pointer) *_pointer = this->encode(*_pointer);
			}

			T*& get()
			{
				return this->decodeRef(*_pointer);
			}
		};

		template <typename T>
		class Pointer : public Cryption<T>
		{
			T* _pointer;

		public:
			Pointer()
			{
				this->_key = 0;
				_pointer = nullptr;
			}

			explicit Pointer(const T*& pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
			}

			explicit Pointer(T*&& pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
			}

			Pointer& operator=(T* pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
				return *this;
			}

			Pointer& operator=(const T*& pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
				return *this;
			}

			void Set(T* pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
			}

			void Set(const T*& pointer)
			{
				this->_key = pointer == nullptr ? 0 : RAND_NUMBER;
				_pointer = pointer == nullptr ? nullptr : this->encode(pointer);
			}

			T value() const { return *this->decode(_pointer); }
			T value() { return *this->decode(_pointer); }
			T* get() const { return this->decode(_pointer); }
			T* get() { return this->decode(_pointer); }
			UniquePointer<T> ref() { return UniquePointer<T>(&_pointer, this->_key); }
			UniquePointer<T> reference() { return ref(); }

			bool valid() const { return (_pointer != nullptr); }
			bool valid() { return (_pointer != nullptr); }

			void free()
			{
				if (_pointer == nullptr) return;
				_pointer = this->decode(_pointer);
				FREE<T>(_pointer);
				_pointer = nullptr;
				this->_key = 0;
			}
		};
	}
}
NET_DSA_END