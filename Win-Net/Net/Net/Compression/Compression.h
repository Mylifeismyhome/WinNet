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
			int Compress(BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION);
			int Compress(BYTE*&, BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION);
			int Decompress(BYTE*&, size_t&);
			int Decompress(BYTE*&, BYTE*&, size_t&);
		}
	}
}
NET_DSA_END