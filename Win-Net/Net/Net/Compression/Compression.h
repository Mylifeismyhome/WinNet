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

#define NET_ZLIB Net::Compression::ZLib

#include <Net/Net/Net.h>
#include <ZLib/zlib.h>

NET_DSA_BEGIN
enum class ZLIB_CompressionLevel
{
	NO_COMPRESSION = Z_NO_COMPRESSION,
	BEST_SPEED = Z_BEST_SPEED,
	BEST_COMPRESSION = Z_BEST_COMPRESSION,
	DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
};

namespace Net
{
	namespace Compression
	{
		namespace ZLib
		{
			int Compress(BYTE* m_pUncompressed, size_t m_iSizeUncompressed, BYTE*& m_pCompressed, size_t& m_iSizeCompressed, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION);
			int Decompress(BYTE* m_pCompressed, size_t m_iSizeCompressed, BYTE*& m_pUncompressed, size_t m_iSizeUncompressed);
		}
	}
}
NET_DSA_END