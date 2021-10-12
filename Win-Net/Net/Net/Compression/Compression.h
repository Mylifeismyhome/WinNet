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

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
NET_NAMESPACE_BEGIN(ZLib)
int Compress(BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION);
int Compress(BYTE*&, BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION, bool = false);
int Decompress(BYTE*&, size_t&, bool = false);
int Decompress(BYTE*&, BYTE*&, size_t&, bool = false);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END