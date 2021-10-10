#pragma once
#include <Net/Net/Net.h>

#define NET_ZLIB Net::Compression::ZLib
#define NET_LZMA Net::Compression::LZMA

#include <ZLib/zlib.h>

NET_DSA_BEGIN

const size_t chunk = 1024;

struct ChunkVector
{
	byte data[chunk];

	explicit ChunkVector(byte* data, const int err, const size_t listSize, const size_t totalOut)
	{
		memset(this->data, NULL, chunk);
		memcpy(this->data, data, err == Z_STREAM_END ? totalOut - listSize * chunk : chunk);
	}
};

enum class ZLIB_CompressionLevel
{
	NO_COMPRESSION = Z_NO_COMPRESSION,
	BEST_SPEED = Z_BEST_SPEED,
	BEST_COMPRESSION = Z_BEST_COMPRESSION,
	DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
};

enum class LZMA_CompressionLevel
{
	ELZMA_lzip,
	ELZMA_lzma
};

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
NET_NAMESPACE_BEGIN(LZMA)
int Compress(BYTE*&, size_t&, LZMA_CompressionLevel = LZMA_CompressionLevel::ELZMA_lzma);
int Compress(BYTE*&, BYTE*&, size_t&, LZMA_CompressionLevel = LZMA_CompressionLevel::ELZMA_lzma, bool = false);
int Decompress(BYTE*&, size_t&, LZMA_CompressionLevel = LZMA_CompressionLevel::ELZMA_lzma);
int Decompress(BYTE*&, BYTE*&, size_t&, LZMA_CompressionLevel = LZMA_CompressionLevel::ELZMA_lzma, bool = false);
NET_NAMESPACE_END
NET_NAMESPACE_BEGIN(ZLib)
int Compress(BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION);
int Compress(BYTE*&, BYTE*&, size_t&, ZLIB_CompressionLevel = ZLIB_CompressionLevel::BEST_COMPRESSION, bool = false);
int Decompress(BYTE*&, size_t&, bool = false);
int Decompress(BYTE*&, BYTE*&, size_t&, bool = false);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END