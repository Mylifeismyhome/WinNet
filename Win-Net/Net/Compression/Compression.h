#pragma once
#include <Net/Net.h>

#define NET_ZLIB Net::Compression::ZLib

#include <ZLib/zlib.h>

NET_DSA_BEGIN

enum class CompressionLevel
{
	NO_COMPRESSION = Z_NO_COMPRESSION,
	BEST_SPEED = Z_BEST_SPEED,
	BEST_COMPRESSION = Z_BEST_COMPRESSION,
	DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION
};

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Compression)
NET_CLASS_BEGIN(ZLib)
NET_CLASS_PUBLIC
int Compress(BYTE*&, size_t&, CompressionLevel = CompressionLevel::BEST_COMPRESSION) const;
int Decompress(BYTE*&, size_t&) const;
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END