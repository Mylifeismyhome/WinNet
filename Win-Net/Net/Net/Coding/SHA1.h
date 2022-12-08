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
#include <Net/Cryption/XOR.h>

#define NET_SHA1 Net::Coding::SHA1
#define NET_SHA1_HEX_SIZE (40 + 1)
#define NET_SHA1_BASE64_SIZE (28 + 1)

NET_DSA_BEGIN
namespace Net
{
	namespace Coding
	{
		class SHA1
		{
			uint32_t state[5];
			uint8_t buf[64];
			uint32_t i;
			uint64_t n_bits;

			void add_byte_dont_count_bits(uint8_t);
			void process_block(const uint8_t*);

		public:
			SHA1(const char* = NULL);

			SHA1& add(uint8_t);
			SHA1& add(char);

			SHA1& add(const void*, uint32_t);
			SHA1& add(const char*);
			SHA1& finalize();

			const SHA1& to_hex(char*, bool = true, const char* = CSTRING("0123456789abcdef"));
			const SHA1& to_base64(char*, bool = true);
		};
	}
}
NET_DSA_END